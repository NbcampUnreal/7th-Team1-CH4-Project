#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <cstdint>

class IOCPServer;
class Session;
class PacketHandler;
class RedisClient;

// ============================================================
//  Registered dedicated server info
// ============================================================

struct DediServerInfo
{
    uint64_t    SessionId;   // IOCP session ID for this dedi server
    std::string ServerId;    // unique ID (generated on register)
    std::string IP;
    uint16_t    Port;
    int         MaxPlayers;
    std::string Status;      // "idle", "ingame"
    int         PlayerCount;
};

// ============================================================
//  DediPool
//  - Manages dedicated server registration, heartbeat, assignment
//  - Handles DEDI_REGISTER, DEDI_HEARTBEAT, DEDI_MATCH_RESULT
// ============================================================

class DediPool
{
public:
    DediPool(IOCPServer& server, RedisClient& redis);

    // Register packet handlers
    void Register(PacketHandler& handler);

    // Find an idle dedicated server and assign it to a match
    // Returns nullptr if no server available
    DediServerInfo* AssignServer(const std::string& matchId);

    // Called when a dedi server session disconnects
    void OnSessionDisconnected(uint64_t sessionId);

    // Get server info by session ID
    DediServerInfo* FindBySessionId(uint64_t sessionId);

    // Send DEDI_ASSIGN_MATCH to the dedi server
    void SendAssignMatch(const DediServerInfo& dedi, const std::string& matchId,
                         const std::string& playersJson, const std::string& authTokensJson);

private:
    void HandleDediRegister(std::shared_ptr<Session> session, const std::string& jsonBody);
    void HandleDediHeartbeat(std::shared_ptr<Session> session, const std::string& jsonBody);
    void HandleDediMatchResult(std::shared_ptr<Session> session, const std::string& jsonBody);

    static std::string GenerateServerId();

    IOCPServer&  m_Server;
    RedisClient& m_Redis;

    std::mutex m_Mutex;
    std::unordered_map<uint64_t, DediServerInfo> m_Servers; // key: sessionId
};
