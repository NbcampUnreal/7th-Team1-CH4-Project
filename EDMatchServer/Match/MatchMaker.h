#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <cstdint>

class IOCPServer;
class Session;
class PacketHandler;
class RedisClient;
class TokenManager;
class MatchQueue;
class MySQLPool;
struct QueueEntry;

// ============================================================
//  MatchMaker
//  - Runs a background tick thread (1 second interval)
//  - Scans queue, groups 6 players by rating proximity
//  - Creates match (lobby in Redis), sends MATCH_FOUND
//  - Handles MATCH_QUEUE_REQ / MATCH_CANCEL packets
// ============================================================

class MatchMaker
{
public:
    MatchMaker(IOCPServer& server, RedisClient& redis,
               TokenManager& tokenMgr, MySQLPool& mysql);
    ~MatchMaker();

    // Register packet handlers (MATCH_QUEUE_REQ, MATCH_CANCEL)
    void Register(PacketHandler& handler);

    // Start/stop the tick thread
    void Start();
    void Stop();

    // Called when a session disconnects (remove from queue)
    void OnSessionDisconnected(uint64_t sessionId);

    // Configuration
    static constexpr int PLAYERS_PER_MATCH = 6;

private:
    void HandleMatchQueueReq(std::shared_ptr<Session> session, const std::string& jsonBody);
    void HandleMatchCancel(std::shared_ptr<Session> session, const std::string& jsonBody);

    // Background tick
    void TickThread();
    void Tick();

    // Try to form a match from current queue entries
    bool TryFormMatch(std::vector<QueueEntry>& entries);

    // Create match: store lobby in Redis, notify players
    void CreateMatch(const std::vector<QueueEntry>& matchedPlayers);

    // Generate UUID match ID
    static std::string GenerateMatchId();

    // Calculate allowed rating range based on wait time
    static int GetRatingRange(double waitSeconds);

    IOCPServer&   m_Server;
    RedisClient&  m_Redis;
    TokenManager& m_TokenMgr;
    MySQLPool&    m_MySQL;

    std::unique_ptr<MatchQueue> m_Queue;

    std::thread       m_TickThread;
    std::atomic<bool> m_Running;
};
