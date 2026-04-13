#pragma once

#include "../Common/Platform.h"

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <cstdint>

struct redisContext;

// ============================================================
//  Redis client wrapper (hiredis)
//  Thread-safe: all commands go through a mutex
// ============================================================

class RedisClient
{
public:
    RedisClient();
    ~RedisClient();

    bool Connect(const std::string& host = "127.0.0.1", uint16_t port = 6379);
    void Disconnect();
    bool IsConnected() const;

    // ---------- String commands ----------
    bool Set(const std::string& key, const std::string& value);
    bool SetEx(const std::string& key, const std::string& value, int ttlSeconds);
    std::string Get(const std::string& key);
    bool Del(const std::string& key);
    bool Exists(const std::string& key);

    // ---------- Hash commands ----------
    bool HSet(const std::string& key, const std::string& field, const std::string& value);
    std::string HGet(const std::string& key, const std::string& field);
    bool HDel(const std::string& key, const std::string& field);
    std::vector<std::pair<std::string, std::string>> HGetAll(const std::string& key);

    // ---------- Sorted Set commands ----------
    bool ZAdd(const std::string& key, double score, const std::string& member);
    bool ZRem(const std::string& key, const std::string& member);
    std::vector<std::string> ZRangeByScore(const std::string& key, double min, double max);
    std::vector<std::pair<std::string, double>> ZRangeByScoreWithScores(const std::string& key, double min, double max);
    int64_t ZCard(const std::string& key);

private:
    redisContext* m_Context;
    std::mutex m_Mutex;
};
