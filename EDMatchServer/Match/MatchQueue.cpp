#include "MatchQueue.h"
#include "../DB/RedisClient.h"
#include "../Auth/TokenManager.h"
#include "../Common/Logger.h"

#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

MatchQueue::MatchQueue(RedisClient& redis, TokenManager& tokenMgr)
    : m_Redis(redis)
    , m_TokenMgr(tokenMgr)
{
}

bool MatchQueue::Enqueue(uint64_t sessionId, uint64_t userId,
                          const std::string& nickname, int rating)
{
    // Store sessionId -> member mapping for dequeue
    std::string memberJson = MakeMemberJson(sessionId, userId, nickname, rating);
    std::string sessionKey = MakeSessionKey(sessionId);

    // Save member JSON in a separate key so we can look it up by sessionId
    m_Redis.Set(sessionKey, memberJson);

    // Score = current epoch time in seconds (for wait-time calculation)
    auto now = std::chrono::system_clock::now();
    double score = std::chrono::duration<double>(now.time_since_epoch()).count();

    bool ok = m_Redis.ZAdd(QUEUE_KEY, score, memberJson);
    if (ok)
    {
        LOG_INFO("[MatchQueue] Enqueued: userId=%llu, nickname=%s, rating=%d",
                 userId, nickname.c_str(), rating);
    }
    return ok;
}

bool MatchQueue::Dequeue(uint64_t sessionId)
{
    std::string sessionKey = MakeSessionKey(sessionId);
    std::string memberJson = m_Redis.Get(sessionKey);

    if (memberJson.empty())
        return false;

    bool ok = m_Redis.ZRem(QUEUE_KEY, memberJson);
    m_Redis.Del(sessionKey);

    if (ok)
    {
        LOG_INFO("[MatchQueue] Dequeued: sessionId=%llu", sessionId);
    }
    return ok;
}

bool MatchQueue::IsInQueue(uint64_t sessionId) const
{
    std::string sessionKey = MakeSessionKey(sessionId);
    return m_Redis.Exists(sessionKey);
}

std::vector<QueueEntry> MatchQueue::GetAllEntries()
{
    std::vector<QueueEntry> entries;

    auto rawEntries = m_Redis.ZRangeByScoreWithScores(QUEUE_KEY, 0, std::numeric_limits<double>::max());

    for (auto& [memberJson, score] : rawEntries)
    {
        QueueEntry entry;
        if (ParseMemberJson(memberJson, entry))
        {
            entry.EnqueueTime = score;
            entries.push_back(std::move(entry));
        }
    }

    return entries;
}

void MatchQueue::RemoveEntries(const std::vector<uint64_t>& sessionIds)
{
    for (uint64_t sid : sessionIds)
    {
        Dequeue(sid);
    }
}

int64_t MatchQueue::GetQueueSize()
{
    return m_Redis.ZCard(QUEUE_KEY);
}

// ============================================================
//  Helpers
// ============================================================

std::string MatchQueue::MakeMemberJson(uint64_t sessionId, uint64_t userId,
                                        const std::string& nickname, int rating)
{
    json j;
    j["session_id"] = sessionId;
    j["user_id"]    = userId;
    j["nickname"]   = nickname;
    j["rating"]     = rating;
    return j.dump();
}

bool MatchQueue::ParseMemberJson(const std::string& jsonStr, QueueEntry& out)
{
    try
    {
        json j = json::parse(jsonStr);
        out.SessionId = j.value("session_id", (uint64_t)0);
        out.UserId    = j.value("user_id", (uint64_t)0);
        out.Nickname  = j.value("nickname", "");
        out.Rating    = j.value("rating", 1000);
        return out.SessionId != 0;
    }
    catch (...)
    {
        LOG_ERROR("[MatchQueue] Failed to parse member JSON: %s", jsonStr.c_str());
        return false;
    }
}

std::string MatchQueue::MakeSessionKey(uint64_t sessionId) const
{
    return "matchqueue:session:" + std::to_string(sessionId);
}
