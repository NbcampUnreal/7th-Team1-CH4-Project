#include "DediPool.h"
#include "../Core/IOCPServer.h"
#include "../Core/Session.h"
#include "../Core/PacketHandler.h"
#include "../Common/Protocol.h"
#include "../Common/Packet.h"
#include "../Common/Logger.h"
#include "../DB/RedisClient.h"

#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

// ============================================================
//  Constructor
// ============================================================

DediPool::DediPool(IOCPServer& server, RedisClient& redis)
    : m_Server(server)
    , m_Redis(redis)
{
}

// ============================================================
//  Register packet handlers
// ============================================================

void DediPool::Register(PacketHandler& handler)
{
    handler.Register(Protocol::D2S_DEDI_REGISTER,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleDediRegister(session, body);
        });

    handler.Register(Protocol::D2S_DEDI_HEARTBEAT,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleDediHeartbeat(session, body);
        });

    handler.Register(Protocol::D2S_DEDI_MATCH_RESULT,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleDediMatchResult(session, body);
        });
}

// ============================================================
//  DEDI_REGISTER
// ============================================================

void DediPool::HandleDediRegister(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] DEDI_REGISTER: %s", session->GetId(), jsonBody.c_str());

    json req;
    try { req = json::parse(jsonBody); } catch (...) { return; }

    DediServerInfo info;
    info.SessionId   = session->GetId();
    info.ServerId    = GenerateServerId();
    info.IP          = req.value("ip", "");
    info.Port        = static_cast<uint16_t>(req.value("port", 7777));
    info.MaxPlayers  = req.value("max_players", 6);
    info.Status      = "idle";
    info.PlayerCount = 0;

    if (info.IP.empty())
    {
        LOG_WARN("[DediPool] DEDI_REGISTER missing IP");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Servers[session->GetId()] = info;
    }

    // Store in Redis for status tracking
    std::string redisKey = "dediserver:" + info.ServerId;
    m_Redis.HSet(redisKey, "ip", info.IP);
    m_Redis.HSet(redisKey, "port", std::to_string(info.Port));
    m_Redis.HSet(redisKey, "status", "idle");
    m_Redis.HSet(redisKey, "session_id", std::to_string(session->GetId()));

    LOG_INFO("[DediPool] Registered: %s -> %s:%d (session %llu)",
             info.ServerId.c_str(), info.IP.c_str(), info.Port, session->GetId());
}

// ============================================================
//  DEDI_HEARTBEAT
// ============================================================

void DediPool::HandleDediHeartbeat(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    json req;
    try { req = json::parse(jsonBody); } catch (...) { return; }

    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Servers.find(session->GetId());
    if (it == m_Servers.end()) return;

    it->second.Status      = req.value("status", it->second.Status);
    it->second.PlayerCount = req.value("player_count", it->second.PlayerCount);

    // Update Redis
    std::string redisKey = "dediserver:" + it->second.ServerId;
    m_Redis.HSet(redisKey, "status", it->second.Status);
    m_Redis.HSet(redisKey, "player_count", std::to_string(it->second.PlayerCount));
}

// ============================================================
//  DEDI_MATCH_RESULT
// ============================================================

void DediPool::HandleDediMatchResult(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] DEDI_MATCH_RESULT: %s", session->GetId(), jsonBody.c_str());

    // Phase 8 will handle match_history recording and rating updates.
    // For now, just mark the dedi server as idle again.
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Servers.find(session->GetId());
        if (it != m_Servers.end())
        {
            it->second.Status = "idle";
            it->second.PlayerCount = 0;

            std::string redisKey = "dediserver:" + it->second.ServerId;
            m_Redis.HSet(redisKey, "status", "idle");
            m_Redis.HSet(redisKey, "player_count", "0");

            LOG_INFO("[DediPool] Server %s back to idle after match",
                     it->second.ServerId.c_str());
        }
    }
}

// ============================================================
//  Assign server to match
// ============================================================

DediServerInfo* DediPool::AssignServer(const std::string& matchId)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& [sessionId, info] : m_Servers)
    {
        if (info.Status == "idle")
        {
            info.Status = "ingame";

            // Update Redis
            std::string redisKey = "dediserver:" + info.ServerId;
            m_Redis.HSet(redisKey, "status", "ingame");
            m_Redis.HSet(redisKey, "match_id", matchId);

            LOG_INFO("[DediPool] Assigned server %s (%s:%d) to match %s",
                     info.ServerId.c_str(), info.IP.c_str(), info.Port, matchId.c_str());
            return &info;
        }
    }

    LOG_WARN("[DediPool] No idle server available for match %s", matchId.c_str());
    return nullptr;
}

// ============================================================
//  Session disconnect
// ============================================================

void DediPool::OnSessionDisconnected(uint64_t sessionId)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Servers.find(sessionId);
    if (it == m_Servers.end()) return;

    std::string redisKey = "dediserver:" + it->second.ServerId;
    m_Redis.Del(redisKey);

    LOG_INFO("[DediPool] Server %s disconnected (%s:%d)",
             it->second.ServerId.c_str(), it->second.IP.c_str(), it->second.Port);

    m_Servers.erase(it);
}

// ============================================================
//  Send DEDI_ASSIGN_MATCH
// ============================================================

void DediPool::SendAssignMatch(const DediServerInfo& dedi, const std::string& matchId,
                                const std::string& playersJson, const std::string& authTokensJson)
{
    json body;
    body["match_id"]    = matchId;
    body["players"]     = json::parse(playersJson);
    body["auth_tokens"] = json::parse(authTokensJson);

    auto packet = Packet::Build(Protocol::S2D_DEDI_ASSIGN, body.dump());

    auto session = m_Server.FindSession(dedi.SessionId);
    if (session)
    {
        session->Send(packet);
        LOG_INFO("[DediPool] Sent DEDI_ASSIGN_MATCH to %s for match %s",
                 dedi.ServerId.c_str(), matchId.c_str());
    }
}

// ============================================================
//  Helpers
// ============================================================

DediServerInfo* DediPool::FindBySessionId(uint64_t sessionId)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Servers.find(sessionId);
    if (it != m_Servers.end())
        return &it->second;
    return nullptr;
}

std::string DediPool::GenerateServerId()
{
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    std::ostringstream ss;
    ss << "dedi-" << std::hex << std::setfill('0') << std::setw(8) << dist(rng);
    return ss.str();
}
