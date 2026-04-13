#pragma once

#include <string>
#include <vector>
#include <cstdint>

class RedisClient;
class TokenManager;

// ============================================================
//  Queue entry (parsed from Redis member JSON)
// ============================================================

struct QueueEntry
{
    uint64_t    SessionId;
    uint64_t    UserId;
    std::string Nickname;
    int         Rating;
    double      EnqueueTime; // epoch seconds (score in sorted set)
};

// ============================================================
//  MatchQueue
//  - Manages the matchmaking queue via Redis Sorted Set
//  - Key: "matchqueue", Score: enqueue timestamp, Member: JSON
// ============================================================

class MatchQueue
{
public:
    MatchQueue(RedisClient& redis, TokenManager& tokenMgr);

    // Add player to queue. Returns true on success.
    bool Enqueue(uint64_t sessionId, uint64_t userId, const std::string& nickname, int rating);

    // Remove player from queue by sessionId
    bool Dequeue(uint64_t sessionId);

    // Check if a session is already in queue
    bool IsInQueue(uint64_t sessionId) const;

    // Get all queue entries sorted by enqueue time
    std::vector<QueueEntry> GetAllEntries();

    // Remove multiple entries by sessionId (after match formed)
    void RemoveEntries(const std::vector<uint64_t>& sessionIds);

    // Get queue size
    int64_t GetQueueSize();

private:
    static std::string MakeMemberJson(uint64_t sessionId, uint64_t userId,
                                       const std::string& nickname, int rating);
    static bool ParseMemberJson(const std::string& json, QueueEntry& out);

    // Build Redis member key for lookup: "session:{sessionId}"
    std::string MakeSessionKey(uint64_t sessionId) const;

    RedisClient&  m_Redis;
    TokenManager& m_TokenMgr;

    static constexpr const char* QUEUE_KEY = "matchqueue";
};
