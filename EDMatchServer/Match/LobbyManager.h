#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

class IOCPServer;
class Session;
class PacketHandler;
class RedisClient;
class TokenManager;
class DediPool;

// ============================================================
//  LobbyManager
//  - Manages post-matchmaking lobby state (team, ready)
//  - Lobby data stored in Redis: lobby:{match_id}
//  - Broadcasts LOBBY_STATE on every change
//  - Detects all-ready → assigns dedi server → sends GAME_START
// ============================================================

class LobbyManager
{
public:
    LobbyManager(IOCPServer& server, RedisClient& redis, TokenManager& tokenMgr, DediPool& dediPool);

    // Register packet handlers (LOBBY_READY, LOBBY_TEAM_CHANGE)
    void Register(PacketHandler& handler);

    // Called when a session disconnects (remove from lobby)
    void OnSessionDisconnected(uint64_t sessionId);

private:
    void HandleLobbyReady(std::shared_ptr<Session> session, const std::string& jsonBody);
    void HandleLobbyTeamChange(std::shared_ptr<Session> session, const std::string& jsonBody);

    // Broadcast current lobby state to all players in the lobby
    void BroadcastLobbyState(const std::string& matchId);

    // All players ready → assign dedi → send GAME_START
    void StartGame(const std::string& matchId);

    // Get lobby JSON from Redis
    std::string GetLobbyJson(const std::string& matchId);

    // Save lobby JSON to Redis
    void SaveLobbyJson(const std::string& matchId, const std::string& json);

    IOCPServer&   m_Server;
    RedisClient&  m_Redis;
    TokenManager& m_TokenMgr;
    DediPool&     m_DediPool;
};
