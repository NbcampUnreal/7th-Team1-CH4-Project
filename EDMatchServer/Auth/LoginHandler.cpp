#include "LoginHandler.h"
#include "TokenManager.h"
#include "../Core/PacketHandler.h"
#include "../Core/Session.h"
#include "../Common/Protocol.h"
#include "../Common/Packet.h"
#include "../Common/Logger.h"
#include "../DB/MySQLPool.h"

#include <nlohmann/json.hpp>

#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

LoginHandler::LoginHandler(MySQLPool& mysql, TokenManager& tokenMgr)
    : m_MySQL(mysql)
    , m_TokenMgr(tokenMgr)
{
}

void LoginHandler::Register(PacketHandler& handler)
{
    handler.Register(Protocol::C2S_LOGIN_REQ,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleLogin(session, body);
        });

    handler.Register(Protocol::C2S_REGISTER_REQ,
        [this](std::shared_ptr<Session> session, const std::string& body)
        {
            HandleRegister(session, body);
        });
}

void LoginHandler::HandleLogin(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] LOGIN_REQ: %s", session->GetId(), jsonBody.c_str());

    // Parse JSON
    json req;
    try
    {
        req = json::parse(jsonBody);
    }
    catch (...)
    {
        LOG_ERROR("Session[%llu] Invalid JSON in LOGIN_REQ", session->GetId());
        json res = { {"result", "fail"}, {"reason", "invalid_json"} };
        session->Send(Packet::Build(Protocol::S2C_LOGIN_RES, res.dump()));
        return;
    }

    std::string loginId = req.value("id", "");
    std::string password = req.value("pw", "");

    if (loginId.empty() || password.empty())
    {
        json res = { {"result", "fail"}, {"reason", "empty_field"} };
        session->Send(Packet::Build(Protocol::S2C_LOGIN_RES, res.dump()));
        return;
    }

    // Hash password
    std::string pwHash = SHA256Hash(password);

    // Query MySQL
    std::string escaped = m_MySQL.Escape(loginId);
    std::string sql = "SELECT id, nickname, pw_hash FROM accounts WHERE login_id = '" + escaped + "'";

    auto rows = m_MySQL.Query(sql);

    if (rows.empty())
    {
        LOG_WARN("Session[%llu] Login failed: account not found [%s]", session->GetId(), loginId.c_str());
        json res = { {"result", "fail"}, {"reason", "invalid_account"} };
        session->Send(Packet::Build(Protocol::S2C_LOGIN_RES, res.dump()));
        return;
    }

    // rows[0]: [0]=id, [1]=nickname, [2]=pw_hash
    std::string dbId       = rows[0][0];
    std::string dbNickname = rows[0][1];
    std::string dbPwHash   = rows[0][2];

    if (pwHash != dbPwHash)
    {
        LOG_WARN("Session[%llu] Login failed: wrong password [%s]", session->GetId(), loginId.c_str());
        json res = { {"result", "fail"}, {"reason", "wrong_password"} };
        session->Send(Packet::Build(Protocol::S2C_LOGIN_RES, res.dump()));
        return;
    }

    // Issue token
    uint64_t userId = std::stoull(dbId);
    std::string token = m_TokenMgr.CreateToken(userId, dbNickname);

    if (token.empty())
    {
        json res = { {"result", "fail"}, {"reason", "token_error"} };
        session->Send(Packet::Build(Protocol::S2C_LOGIN_RES, res.dump()));
        return;
    }

    // Update session info
    session->SetUserId(userId);
    session->SetNickname(dbNickname);
    session->SetAuthToken(token);

    LOG_INFO("Session[%llu] Login OK: userId=%llu, nickname=%s", session->GetId(), userId, dbNickname.c_str());

    json res = {
        {"result", "ok"},
        {"token", token},
        {"nickname", dbNickname}
    };
    session->Send(Packet::Build(Protocol::S2C_LOGIN_RES, res.dump()));
}

void LoginHandler::HandleRegister(std::shared_ptr<Session> session, const std::string& jsonBody)
{
    LOG_INFO("Session[%llu] REGISTER_REQ: %s", session->GetId(), jsonBody.c_str());

    json req;
    try
    {
        req = json::parse(jsonBody);
    }
    catch (...)
    {
        LOG_ERROR("Session[%llu] Invalid JSON in REGISTER_REQ", session->GetId());
        json res = { {"result", "fail"}, {"reason", "invalid_json"} };
        session->Send(Packet::Build(Protocol::S2C_REGISTER_RES, res.dump()));
        return;
    }

    std::string loginId  = req.value("id", "");
    std::string password = req.value("pw", "");
    std::string nickname = req.value("nickname", "");

    if (loginId.empty() || password.empty() || nickname.empty())
    {
        json res = { {"result", "fail"}, {"reason", "empty_field"} };
        session->Send(Packet::Build(Protocol::S2C_REGISTER_RES, res.dump()));
        return;
    }

    // ID 중복 체크
    std::string escapedId = m_MySQL.Escape(loginId);
    std::string checkSql = "SELECT id FROM accounts WHERE login_id = '" + escapedId + "'";
    auto existing = m_MySQL.Query(checkSql);

    if (!existing.empty())
    {
        LOG_WARN("Session[%llu] Register failed: duplicate id [%s]", session->GetId(), loginId.c_str());
        json res = { {"result", "fail"}, {"reason", "duplicate_id"} };
        session->Send(Packet::Build(Protocol::S2C_REGISTER_RES, res.dump()));
        return;
    }

    // 닉네임 중복 체크
    std::string escapedNick = m_MySQL.Escape(nickname);
    std::string checkNickSql = "SELECT id FROM accounts WHERE nickname = '" + escapedNick + "'";
    auto existingNick = m_MySQL.Query(checkNickSql);

    if (!existingNick.empty())
    {
        LOG_WARN("Session[%llu] Register failed: duplicate nickname [%s]", session->GetId(), nickname.c_str());
        json res = { {"result", "fail"}, {"reason", "duplicate_nickname"} };
        session->Send(Packet::Build(Protocol::S2C_REGISTER_RES, res.dump()));
        return;
    }

    // 계정 생성
    std::string pwHash = SHA256Hash(password);
    std::string insertSql = "INSERT INTO accounts (login_id, nickname, pw_hash) VALUES ('"
        + escapedId + "', '" + escapedNick + "', '" + pwHash + "')";

    if (!m_MySQL.Execute(insertSql))
    {
        LOG_ERROR("Session[%llu] Register failed: DB insert error", session->GetId());
        json res = { {"result", "fail"}, {"reason", "db_error"} };
        session->Send(Packet::Build(Protocol::S2C_REGISTER_RES, res.dump()));
        return;
    }

    LOG_INFO("Session[%llu] Register OK: %s (%s)", session->GetId(), loginId.c_str(), nickname.c_str());
    json res = { {"result", "ok"} };
    session->Send(Packet::Build(Protocol::S2C_REGISTER_RES, res.dump()));
}

std::string LoginHandler::SHA256Hash(const std::string& input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}
