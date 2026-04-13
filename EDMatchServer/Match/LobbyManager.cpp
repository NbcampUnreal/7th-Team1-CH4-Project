#include "LobbyManager.h"
#include "../Core/IOCPServer.h"
#include "../Core/Session.h"
#include "../Core/PacketHandler.h"
#include "../Common/Protocol.h"
#include "../Common/Packet.h"
#include "../Common/Logger.h"
#include "../DB/RedisClient.h"
#include "../Auth/TokenManager.h"
#include "../DediServer/DediPool.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ============================================================
//  Constructor
// ============================================================

LobbyManager::LobbyManager(IOCPServer& server, RedisClient& redis,
                             TokenManager& tokenMgr, DediPool& dediPool)
    : m_Server(server)
    , m_Redis(redis)
    , m_TokenMgr(tokenMgr)
    , m_DediPool(dediPool)
{
}

// ============================================================
//  Register packet handlers
// ============================================================

void LobbyManager::Register(PacketHandler& handler)
{
    handler.Register(Protocol::C2S_LOBBY_READY,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleLobbyReady(session, body);
        });

    handler.Register(Protocol::C2S_LOBBY_TEAM_CHANGE,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleLobbyTeamChange(session, body);
        });
}

// ============================================================
//  LOBBY_READY handler
// ============================================================

void LobbyManager::HandleLobbyReady(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] LOBBY_READY", session->GetId());

    json req;
    try { req = json::parse(jsonBody); } catch (...) { return; }

    std::string token   = req.value("token", "");
    std::string matchId = req.value("match_id", "");

    if (token.empty() || matchId.empty()) return;
    if (m_TokenMgr.ValidateToken(token) == 0) return;

    // Load lobby state
    std::string lobbyJson = GetLobbyJson(matchId);
    if (lobbyJson.empty())
    {
        LOG_WARN("[LobbyManager] Lobby not found: %s", matchId.c_str());
        return;
    }

    json lobby;
    try { lobby = json::parse(lobbyJson); } catch (...) { return; }

    // Don't allow changes if game is already starting
    if (lobby.value("status", "") == "starting")
    {
        LOG_WARN("[LobbyManager] Game already starting for match %s", matchId.c_str());
        return;
    }

    // Toggle ready state for this player
    bool found = false;
    for (auto& player : lobby["players"])
    {
        if (player["session_id"].get<uint64_t>() == session->GetId())
        {
            bool currentReady = player.value("ready", false);
            player["ready"] = !currentReady;
            found = true;
            LOG_INFO("[LobbyManager] %s ready: %s -> %s (match: %s)",
                     session->GetNickname().c_str(),
                     currentReady ? "true" : "false",
                     !currentReady ? "true" : "false",
                     matchId.c_str());
            break;
        }
    }

    if (!found)
    {
        LOG_WARN("[LobbyManager] Player not in lobby: session=%llu, match=%s",
                 session->GetId(), matchId.c_str());
        return;
    }

    // Check if all ready
    bool allReady = true;
    for (auto& player : lobby["players"])
    {
        if (!player.value("ready", false))
        {
            allReady = false;
            break;
        }
    }

    if (allReady)
    {
        lobby["status"] = "all_ready";
        LOG_INFO("[LobbyManager] === ALL READY: match %s ===", matchId.c_str());
    }
    else
    {
        lobby["status"] = "waiting";
    }

    // Save and broadcast
    SaveLobbyJson(matchId, lobby.dump());
    BroadcastLobbyState(matchId);

    // All ready → assign dedi server → send GAME_START
    if (allReady)
    {
        StartGame(matchId);
    }
}

// ============================================================
//  LOBBY_TEAM_CHANGE handler
// ============================================================

void LobbyManager::HandleLobbyTeamChange(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] LOBBY_TEAM_CHANGE", session->GetId());

    json req;
    try { req = json::parse(jsonBody); } catch (...) { return; }

    std::string token   = req.value("token", "");
    std::string matchId = req.value("match_id", "");
    int newTeam         = req.value("team", -1);

    if (token.empty() || matchId.empty() || newTeam < 0) return;
    if (m_TokenMgr.ValidateToken(token) == 0) return;

    // Load lobby state
    std::string lobbyJson = GetLobbyJson(matchId);
    if (lobbyJson.empty()) return;

    json lobby;
    try { lobby = json::parse(lobbyJson); } catch (...) { return; }

    // Update team for this player
    bool found = false;
    for (auto& player : lobby["players"])
    {
        if (player["session_id"].get<uint64_t>() == session->GetId())
        {
            int oldTeam = player.value("team", 0);
            player["team"] = newTeam;
            found = true;
            LOG_INFO("[LobbyManager] %s team: %d -> %d (match: %s)",
                     session->GetNickname().c_str(), oldTeam, newTeam, matchId.c_str());
            break;
        }
    }

    if (!found) return;

    // Save and broadcast
    SaveLobbyJson(matchId, lobby.dump());
    BroadcastLobbyState(matchId);
}

// ============================================================
//  Broadcast LOBBY_STATE to all players in the lobby
// ============================================================

void LobbyManager::BroadcastLobbyState(const std::string& matchId)
{
    std::string lobbyJson = GetLobbyJson(matchId);
    if (lobbyJson.empty()) return;

    json lobby;
    try { lobby = json::parse(lobbyJson); } catch (...) { return; }

    // Build LOBBY_STATE body (only send what clients need)
    json stateBody;
    stateBody["match_id"] = matchId;
    stateBody["status"]   = lobby.value("status", "waiting");

    json playerList = json::array();
    for (auto& player : lobby["players"])
    {
        json pl;
        pl["nickname"] = player.value("nickname", "");
        pl["team"]     = player.value("team", 0);
        pl["ready"]    = player.value("ready", false);
        pl["rating"]   = player.value("rating", 1000);
        playerList.push_back(pl);
    }
    stateBody["players"] = playerList;

    auto packet = Packet::Build(Protocol::S2C_LOBBY_STATE, stateBody.dump());

    // Send to each player in the lobby
    for (auto& player : lobby["players"])
    {
        uint64_t sessionId = player.value("session_id", (uint64_t)0);
        auto session = m_Server.FindSession(sessionId);
        if (session)
        {
            session->Send(packet);
        }
    }

    LOG_INFO("[LobbyManager] Broadcast LOBBY_STATE to match %s (%zu players)",
             matchId.c_str(), lobby["players"].size());
}

// ============================================================
//  StartGame - all ready → assign dedi → GAME_START
// ============================================================

void LobbyManager::StartGame(const std::string& matchId)
{
    std::string lobbyJson = GetLobbyJson(matchId);
    if (lobbyJson.empty()) return;

    json lobby;
    try { lobby = json::parse(lobbyJson); } catch (...) { return; }

    // Assign a dedi server
    DediServerInfo* dedi = m_DediPool.AssignServer(matchId);
    if (!dedi)
    {
        LOG_ERROR("[LobbyManager] No dedi server available for match %s!", matchId.c_str());

        // Notify players
        json stateBody;
        stateBody["match_id"] = matchId;
        stateBody["status"]   = "no_server";
        stateBody["players"]  = json::array();
        auto packet = Packet::Build(Protocol::S2C_LOBBY_STATE, stateBody.dump());

        for (auto& player : lobby["players"])
        {
            uint64_t sid = player.value("session_id", (uint64_t)0);
            auto session = m_Server.FindSession(sid);
            if (session) session->Send(packet);
        }
        return;
    }

    // Mark lobby as starting
    lobby["status"] = "starting";
    SaveLobbyJson(matchId, lobby.dump());

    // Build player list and auth tokens for dedi server
    json playersForDedi = json::array();
    json authTokens = json::array();
    for (auto& player : lobby["players"])
    {
        json pl;
        pl["user_id"]  = player.value("user_id", (uint64_t)0);
        pl["nickname"] = player.value("nickname", "");
        pl["team"]     = player.value("team", 0);
        playersForDedi.push_back(pl);

        // Get auth token from session
        uint64_t sid = player.value("session_id", (uint64_t)0);
        auto session = m_Server.FindSession(sid);
        if (session)
        {
            authTokens.push_back(session->GetAuthToken());
        }
    }

    // Send DEDI_ASSIGN_MATCH to the dedicated server
    m_DediPool.SendAssignMatch(*dedi, matchId,
                                playersForDedi.dump(), authTokens.dump());

    // Send GAME_START to all players
    json gameStartBody;
    gameStartBody["server_ip"]   = dedi->IP;
    gameStartBody["server_port"] = dedi->Port;
    gameStartBody["match_id"]    = matchId;

    auto gameStartPacket = Packet::Build(Protocol::S2C_GAME_START, gameStartBody.dump());

    for (auto& player : lobby["players"])
    {
        uint64_t sid = player.value("session_id", (uint64_t)0);
        auto session = m_Server.FindSession(sid);
        if (session)
        {
            // Include the player's own auth token for dedi PreLogin
            json personalStart = gameStartBody;
            personalStart["auth_token"] = session->GetAuthToken();
            auto personalPacket = Packet::Build(Protocol::S2C_GAME_START, personalStart.dump());
            session->Send(personalPacket);

            LOG_INFO("[LobbyManager] Sent GAME_START to %s -> %s:%d",
                     session->GetNickname().c_str(), dedi->IP.c_str(), dedi->Port);
        }
    }

    LOG_INFO("[LobbyManager] === GAME STARTING: match %s -> %s:%d ===",
             matchId.c_str(), dedi->IP.c_str(), dedi->Port);

    // Clean up session_lobby mappings
    for (auto& player : lobby["players"])
    {
        uint64_t sid = player.value("session_id", (uint64_t)0);
        m_Redis.Del("session_lobby:" + std::to_string(sid));
    }
}

// ============================================================
//  Session disconnect handling
// ============================================================

void LobbyManager::OnSessionDisconnected(uint64_t sessionId)
{
    std::string matchIdKey = "session_lobby:" + std::to_string(sessionId);
    std::string matchId = m_Redis.Get(matchIdKey);

    if (matchId.empty()) return;

    std::string lobbyJson = GetLobbyJson(matchId);
    if (lobbyJson.empty()) return;

    json lobby;
    try { lobby = json::parse(lobbyJson); } catch (...) { return; }

    // Remove disconnected player from lobby
    std::string nickname;
    auto& players = lobby["players"];
    for (auto it = players.begin(); it != players.end(); ++it)
    {
        if ((*it)["session_id"].get<uint64_t>() == sessionId)
        {
            nickname = (*it).value("nickname", "");
            players.erase(it);
            break;
        }
    }

    m_Redis.Del(matchIdKey);

    if (players.empty())
    {
        std::string lobbyKey = "lobby:" + matchId;
        m_Redis.Del(lobbyKey);
        LOG_INFO("[LobbyManager] Lobby %s dissolved (all disconnected)", matchId.c_str());
    }
    else
    {
        lobby["status"] = "waiting";
        for (auto& p : players)
        {
            p["ready"] = false;
        }

        SaveLobbyJson(matchId, lobby.dump());
        BroadcastLobbyState(matchId);
        LOG_INFO("[LobbyManager] %s left lobby %s (%zu remaining)",
                 nickname.c_str(), matchId.c_str(), players.size());
    }
}

// ============================================================
//  Redis helpers
// ============================================================

std::string LobbyManager::GetLobbyJson(const std::string& matchId)
{
    return m_Redis.Get("lobby:" + matchId);
}

void LobbyManager::SaveLobbyJson(const std::string& matchId, const std::string& jsonStr)
{
    m_Redis.SetEx("lobby:" + matchId, jsonStr, 600);
}
