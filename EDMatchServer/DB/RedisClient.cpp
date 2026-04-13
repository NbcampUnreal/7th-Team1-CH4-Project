#include "RedisClient.h"
#include "../Common/Logger.h"

#include <hiredis/hiredis.h>

RedisClient::RedisClient()
    : m_Context(nullptr)
{
}

RedisClient::~RedisClient()
{
    Disconnect();
}

bool RedisClient::Connect(const std::string& host, uint16_t port)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Context)
    {
        redisFree(m_Context);
        m_Context = nullptr;
    }

    struct timeval timeout = { 3, 0 }; // 3 seconds
    m_Context = redisConnectWithTimeout(host.c_str(), port, timeout);

    if (m_Context == nullptr)
    {
        LOG_ERROR("[Redis] Failed to allocate context");
        return false;
    }

    if (m_Context->err)
    {
        LOG_ERROR("[Redis] Connection error: %s", m_Context->errstr);
        redisFree(m_Context);
        m_Context = nullptr;
        return false;
    }

    LOG_INFO("[Redis] Connected to %s:%d", host.c_str(), port);
    return true;
}

void RedisClient::Disconnect()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Context)
    {
        redisFree(m_Context);
        m_Context = nullptr;
        LOG_INFO("[Redis] Disconnected");
    }
}

bool RedisClient::IsConnected() const
{
    return m_Context != nullptr && m_Context->err == 0;
}

// ============================================================
//  String commands
// ============================================================

bool RedisClient::Set(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context, "SET %s %s", key.c_str(), value.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK");
    freeReplyObject(reply);
    return ok;
}

bool RedisClient::SetEx(const std::string& key, const std::string& value, int ttlSeconds)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "SETEX %s %d %s", key.c_str(), ttlSeconds, value.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK");
    freeReplyObject(reply);
    return ok;
}

std::string RedisClient::Get(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return "";

    redisReply* reply = (redisReply*)redisCommand(m_Context, "GET %s", key.c_str());
    if (!reply) return "";

    std::string result;
    if (reply->type == REDIS_REPLY_STRING)
        result = std::string(reply->str, reply->len);

    freeReplyObject(reply);
    return result;
}

bool RedisClient::Del(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context, "DEL %s", key.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return ok;
}

bool RedisClient::Exists(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context, "EXISTS %s", key.c_str());
    if (!reply) return false;

    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return exists;
}

// ============================================================
//  Hash commands
// ============================================================

bool RedisClient::HSet(const std::string& key, const std::string& field, const std::string& value)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_INTEGER);
    freeReplyObject(reply);
    return ok;
}

std::string RedisClient::HGet(const std::string& key, const std::string& field)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return "";

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "HGET %s %s", key.c_str(), field.c_str());
    if (!reply) return "";

    std::string result;
    if (reply->type == REDIS_REPLY_STRING)
        result = std::string(reply->str, reply->len);

    freeReplyObject(reply);
    return result;
}

bool RedisClient::HDel(const std::string& key, const std::string& field)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "HDEL %s %s", key.c_str(), field.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return ok;
}

std::vector<std::pair<std::string, std::string>> RedisClient::HGetAll(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<std::pair<std::string, std::string>> result;
    if (!m_Context) return result;

    redisReply* reply = (redisReply*)redisCommand(m_Context, "HGETALL %s", key.c_str());
    if (!reply) return result;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements >= 2)
    {
        for (size_t i = 0; i + 1 < reply->elements; i += 2)
        {
            std::string field(reply->element[i]->str, reply->element[i]->len);
            std::string value(reply->element[i + 1]->str, reply->element[i + 1]->len);
            result.emplace_back(std::move(field), std::move(value));
        }
    }

    freeReplyObject(reply);
    return result;
}

// ============================================================
//  Sorted Set commands
// ============================================================

bool RedisClient::ZAdd(const std::string& key, double score, const std::string& member)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "ZADD %s %f %s", key.c_str(), score, member.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_INTEGER);
    freeReplyObject(reply);
    return ok;
}

bool RedisClient::ZRem(const std::string& key, const std::string& member)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return false;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "ZREM %s %s", key.c_str(), member.c_str());
    if (!reply) return false;

    bool ok = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return ok;
}

std::vector<std::string> RedisClient::ZRangeByScore(const std::string& key, double min, double max)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<std::string> result;
    if (!m_Context) return result;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "ZRANGEBYSCORE %s %f %f", key.c_str(), min, max);
    if (!reply) return result;

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            result.emplace_back(reply->element[i]->str, reply->element[i]->len);
        }
    }

    freeReplyObject(reply);
    return result;
}

std::vector<std::pair<std::string, double>> RedisClient::ZRangeByScoreWithScores(const std::string& key, double min, double max)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<std::pair<std::string, double>> result;
    if (!m_Context) return result;

    redisReply* reply = (redisReply*)redisCommand(m_Context,
        "ZRANGEBYSCORE %s %f %f WITHSCORES", key.c_str(), min, max);
    if (!reply) return result;

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements >= 2)
    {
        for (size_t i = 0; i + 1 < reply->elements; i += 2)
        {
            std::string member(reply->element[i]->str, reply->element[i]->len);
            double score = std::stod(std::string(reply->element[i + 1]->str, reply->element[i + 1]->len));
            result.emplace_back(std::move(member), score);
        }
    }

    freeReplyObject(reply);
    return result;
}

int64_t RedisClient::ZCard(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Context) return 0;

    redisReply* reply = (redisReply*)redisCommand(m_Context, "ZCARD %s", key.c_str());
    if (!reply) return 0;

    int64_t count = 0;
    if (reply->type == REDIS_REPLY_INTEGER)
        count = reply->integer;

    freeReplyObject(reply);
    return count;
}
