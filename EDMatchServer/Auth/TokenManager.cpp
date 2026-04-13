#include "TokenManager.h"
#include "../DB/RedisClient.h"
#include "../Common/Logger.h"

#include <random>
#include <sstream>
#include <iomanip>

TokenManager::TokenManager(RedisClient& redis)
    : m_Redis(redis)
{
}

std::string TokenManager::CreateToken(uint64_t userId, const std::string& nickname)
{
    std::string token = GenerateUUID();

    // Store as: session:{token} -> "userId|nickname"
    std::string value = std::to_string(userId) + "|" + nickname;
    std::string key = "session:" + token;

    if (!m_Redis.SetEx(key, value, TOKEN_TTL_SECONDS))
    {
        LOG_ERROR("[TokenManager] Failed to store token for userId=%llu", userId);
        return "";
    }

    LOG_INFO("[TokenManager] Token created for userId=%llu, nickname=%s", userId, nickname.c_str());
    return token;
}

uint64_t TokenManager::ValidateToken(const std::string& token)
{
    std::string key = "session:" + token;
    std::string value = m_Redis.Get(key);

    if (value.empty())
        return 0;

    // Parse "userId|nickname"
    size_t sep = value.find('|');
    if (sep == std::string::npos)
        return 0;

    try
    {
        return std::stoull(value.substr(0, sep));
    }
    catch (...)
    {
        return 0;
    }
}

std::string TokenManager::GetNickname(const std::string& token)
{
    std::string key = "session:" + token;
    std::string value = m_Redis.Get(key);

    if (value.empty())
        return "";

    size_t sep = value.find('|');
    if (sep == std::string::npos)
        return "";

    return value.substr(sep + 1);
}

void TokenManager::RevokeToken(const std::string& token)
{
    std::string key = "session:" + token;
    m_Redis.Del(key);
    LOG_INFO("[TokenManager] Token revoked: %s", token.c_str());
}

std::string TokenManager::GenerateUUID()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dist;

    uint64_t a = dist(gen);
    uint64_t b = dist(gen);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << (a >> 32)        << "-";
    ss << std::setw(4) << ((a >> 16) & 0xFFFF) << "-";
    ss << std::setw(4) << (a & 0xFFFF)     << "-";
    ss << std::setw(4) << (b >> 48)        << "-";
    ss << std::setw(12) << (b & 0xFFFFFFFFFFFF);

    return ss.str();
}
