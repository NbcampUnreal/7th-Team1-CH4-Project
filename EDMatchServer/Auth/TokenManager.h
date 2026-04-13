#pragma once

#include <string>
#include <cstdint>

class RedisClient;

// ============================================================
//  Token Manager
//  - Generates UUID session tokens
//  - Stores/validates tokens via Redis (SETEX with TTL)
// ============================================================

class TokenManager
{
public:
    explicit TokenManager(RedisClient& redis);

    // Generate token, store in Redis with user info, return token string
    std::string CreateToken(uint64_t userId, const std::string& nickname);

    // Validate token, returns userId (0 if invalid/expired)
    uint64_t ValidateToken(const std::string& token);

    // Get nickname from token
    std::string GetNickname(const std::string& token);

    // Remove token (logout)
    void RevokeToken(const std::string& token);

private:
    static std::string GenerateUUID();

    RedisClient& m_Redis;

    static constexpr int TOKEN_TTL_SECONDS = 1800; // 30 min
};
