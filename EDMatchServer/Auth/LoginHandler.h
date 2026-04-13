#pragma once

#include <string>
#include <memory>

class Session;
class MySQLPool;
class RedisClient;
class TokenManager;
class PacketHandler;

// ============================================================
//  Login Handler
//  - Registers LOGIN_REQ packet handler
//  - Flow: parse JSON -> MySQL query -> SHA256 verify -> token issue
// ============================================================

class LoginHandler
{
public:
    LoginHandler(MySQLPool& mysql, TokenManager& tokenMgr);

    // Register handler to PacketHandler
    void Register(PacketHandler& handler);

private:
    void HandleLogin(std::shared_ptr<Session> session, const std::string& jsonBody);

    // SHA256 hash (same as init.sql test data)
    static std::string SHA256Hash(const std::string& input);

    MySQLPool&     m_MySQL;
    TokenManager&  m_TokenMgr;
};
