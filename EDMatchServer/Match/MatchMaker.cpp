#include "MatchMaker.h"
#include "MatchQueue.h"
#include "../Core/IOCPServer.h"
#include "../Core/Session.h"
#include "../Core/PacketHandler.h"
#include "../Common/Protocol.h"
#include "../Common/Packet.h"
#include "../Common/Logger.h"
#include "../DB/RedisClient.h"
#include "../DB/MySQLPool.h"
#include "../Auth/TokenManager.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

// ============================================================
//  Constructor / Destructor
// ============================================================

MatchMaker::MatchMaker(IOCPServer& server, RedisClient& redis,
                       TokenManager& tokenMgr, MySQLPool& mysql)
    : m_Server(server)
    , m_Redis(redis)
    , m_TokenMgr(tokenMgr)
    , m_MySQL(mysql)
    , m_Running(false)
{
    m_Queue = std::make_unique<MatchQueue>(redis, tokenMgr);
}

MatchMaker::~MatchMaker()
{
    Stop();
}

// ============================================================
//  Register packet handlers
// ============================================================

void MatchMaker::Register(PacketHandler& handler)
{
    handler.Register(Protocol::C2S_MATCH_QUEUE_REQ,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleMatchQueueReq(session, body);
        });

    handler.Register(Protocol::C2S_MATCH_CANCEL,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleMatchCancel(session, body);
        });
}

// ============================================================
//  Packet Handlers
// ============================================================

void MatchMaker::HandleMatchQueueReq(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] MATCH_QUEUE_REQ", session->GetId());

    // Must be authenticated
    if (!session->IsAuthenticated())
    {
        LOG_WARN("Session[%llu] Not authenticated for matchmaking", session->GetId());
        json res = { {"result", "fail"}, {"reason", "not_authenticated"} };
        session->Send(Packet::Build(Protocol::S2C_MATCH_QUEUE_RES, res.dump()));
        return;
    }

    // Validate token
    json req;
    try { req = json::parse(jsonBody); } catch (...) { return; }

    std::string token = req.value("token", "");
    if (token.empty() || m_TokenMgr.ValidateToken(token) == 0)
    {
        json res = { {"result", "fail"}, {"reason", "invalid_token"} };
        session->Send(Packet::Build(Protocol::S2C_MATCH_QUEUE_RES, res.dump()));
        return;
    }

    // Check if already in queue
    if (m_Queue->IsInQueue(session->GetId()))
    {
        json res = { {"result", "fail"}, {"reason", "already_queued"} };
        session->Send(Packet::Build(Protocol::S2C_MATCH_QUEUE_RES, res.dump()));
        return;
    }

    // Get player rating from MySQL
    int rating = 1000; // default
    std::string sql = "SELECT rating FROM accounts WHERE id = " + std::to_string(session->GetUserId());
    auto rows = m_MySQL.Query(sql);
    if (!rows.empty() && !rows[0].empty())
    {
        rating = std::stoi(rows[0][0]);
    }

    // Add to queue
    if (m_Queue->Enqueue(session->GetId(), session->GetUserId(), session->GetNickname(), rating))
    {
        json res = { {"result", "queued"} };
        session->Send(Packet::Build(Protocol::S2C_MATCH_QUEUE_RES, res.dump()));

        int64_t queueSize = m_Queue->GetQueueSize();
        LOG_INFO("[MatchMaker] Player queued: %s (rating: %d) | Queue size: %lld",
                 session->GetNickname().c_str(), rating, queueSize);
    }
    else
    {
        json res = { {"result", "fail"}, {"reason", "queue_error"} };
        session->Send(Packet::Build(Protocol::S2C_MATCH_QUEUE_RES, res.dump()));
    }
}

void MatchMaker::HandleMatchCancel(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] MATCH_CANCEL", session->GetId());

    if (m_Queue->Dequeue(session->GetId()))
    {
        LOG_INFO("[MatchMaker] Player dequeued: %s", session->GetNickname().c_str());
    }
}

void MatchMaker::OnSessionDisconnected(uint64_t sessionId)
{
    m_Queue->Dequeue(sessionId);
}

// ============================================================
//  Tick Thread
// ============================================================

void MatchMaker::Start()
{
    if (m_Running.load()) return;

    m_Running.store(true);
    m_TickThread = std::thread(&MatchMaker::TickThread, this);
    LOG_INFO("[MatchMaker] Started (tick interval: 1s, players per match: %d)", PLAYERS_PER_MATCH);
}

void MatchMaker::Stop()
{
    if (!m_Running.exchange(false)) return;

    if (m_TickThread.joinable())
        m_TickThread.join();

    LOG_INFO("[MatchMaker] Stopped");
}

void MatchMaker::TickThread()
{
    while (m_Running.load())
    {
        Tick();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void MatchMaker::Tick()
{
    auto entries = m_Queue->GetAllEntries();

    if (entries.size() < PLAYERS_PER_MATCH)
        return;

    // Sort by rating for proximity matching
    std::sort(entries.begin(), entries.end(),
        [](const QueueEntry& a, const QueueEntry& b) { return a.Rating < b.Rating; });

    // Try to form matches
    while (entries.size() >= PLAYERS_PER_MATCH)
    {
        if (!TryFormMatch(entries))
            break;
    }
}

// ============================================================
//  Match Formation
// ============================================================

bool MatchMaker::TryFormMatch(std::vector<QueueEntry>& entries)
{
    auto now = std::chrono::system_clock::now();
    double nowEpoch = std::chrono::duration<double>(now.time_since_epoch()).count();

    // Sliding window: try to find PLAYERS_PER_MATCH consecutive players
    // whose rating spread fits within the allowed range
    for (size_t i = 0; i + PLAYERS_PER_MATCH <= entries.size(); ++i)
    {
        // Find the minimum allowed range among the candidate group
        // Use the maximum wait time in the group (most generous range)
        double maxWait = 0;
        for (size_t j = i; j < i + PLAYERS_PER_MATCH; ++j)
        {
            double wait = nowEpoch - entries[j].EnqueueTime;
            if (wait > maxWait) maxWait = wait;
        }

        int allowedRange = GetRatingRange(maxWait);
        int ratingSpread = entries[i + PLAYERS_PER_MATCH - 1].Rating - entries[i].Rating;

        if (ratingSpread <= allowedRange * 2) // ±range -> total spread = 2*range
        {
            // Match found!
            std::vector<QueueEntry> matched(entries.begin() + i,
                                             entries.begin() + i + PLAYERS_PER_MATCH);

            // Remove matched entries from the vector
            entries.erase(entries.begin() + i, entries.begin() + i + PLAYERS_PER_MATCH);

            CreateMatch(matched);
            return true;
        }
    }

    return false;
}

int MatchMaker::GetRatingRange(double waitSeconds)
{
    // Expand rating tolerance based on wait time
    //   0~30s:  ±100
    //  30~60s:  ±200
    //  60s+:    ±400
    if (waitSeconds < 30.0)
        return 100;
    else if (waitSeconds < 60.0)
        return 200;
    else
        return 400;
}

// ============================================================
//  Create Match
// ============================================================

void MatchMaker::CreateMatch(const std::vector<QueueEntry>& matchedPlayers)
{
    std::string matchId = GenerateMatchId();

    LOG_INFO("[MatchMaker] === MATCH FORMED: %s ===", matchId.c_str());

    // Remove from queue
    std::vector<uint64_t> sessionIds;
    for (auto& p : matchedPlayers)
    {
        sessionIds.push_back(p.SessionId);
        LOG_INFO("[MatchMaker]   Player: %s (rating: %d)", p.Nickname.c_str(), p.Rating);
    }
    m_Queue->RemoveEntries(sessionIds);

    // Store lobby state in Redis (for Phase 6)
    json lobbyState;
    json playersArray = json::array();
    for (auto& p : matchedPlayers)
    {
        json player;
        player["user_id"]    = p.UserId;
        player["session_id"] = p.SessionId;
        player["nickname"]   = p.Nickname;
        player["rating"]     = p.Rating;
        player["team"]       = 0;  // unassigned
        player["ready"]      = false;
        playersArray.push_back(player);
    }
    lobbyState["players"] = playersArray;
    lobbyState["status"]  = "waiting"; // waiting for ready

    std::string lobbyKey = "lobby:" + matchId;
    m_Redis.Set(lobbyKey, lobbyState.dump());
    // TTL 10 minutes for lobby
    m_Redis.SetEx(lobbyKey, lobbyState.dump(), 600);

    // Build MATCH_FOUND packet
    json matchFoundBody;
    matchFoundBody["match_id"] = matchId;

    json playerList = json::array();
    for (auto& p : matchedPlayers)
    {
        json pl;
        pl["nickname"] = p.Nickname;
        pl["rating"]   = p.Rating;
        playerList.push_back(pl);
    }
    matchFoundBody["players"] = playerList;

    auto packet = Packet::Build(Protocol::S2C_MATCH_FOUND, matchFoundBody.dump());

    // Send MATCH_FOUND to each matched player
    for (auto& p : matchedPlayers)
    {
        auto session = m_Server.FindSession(p.SessionId);
        if (session)
        {
            session->Send(packet);
            LOG_INFO("[MatchMaker] Sent MATCH_FOUND to %s (session %llu)",
                     p.Nickname.c_str(), p.SessionId);
        }
        else
        {
            LOG_WARN("[MatchMaker] Session %llu not found for %s",
                     p.SessionId, p.Nickname.c_str());
        }
    }

    LOG_INFO("[MatchMaker] === MATCH %s: %zu players notified ===",
             matchId.c_str(), matchedPlayers.size());
}

// ============================================================
//  Helpers
// ============================================================

std::string MatchMaker::GenerateMatchId()
{
    // Simple UUID-like match ID
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << dist(rng) << "-";
    ss << std::setw(4) << (dist(rng) & 0xFFFF) << "-";
    ss << std::setw(4) << (dist(rng) & 0xFFFF) << "-";
    ss << std::setw(4) << (dist(rng) & 0xFFFF) << "-";
    ss << std::setw(8) << dist(rng) << std::setw(4) << (dist(rng) & 0xFFFF);
    return ss.str();
}
