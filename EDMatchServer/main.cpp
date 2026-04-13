#include "Common/Platform.h"
#include "Core/IOCPServer.h"
#include "Core/PacketHandler.h"
#include "Core/Session.h"
#include "Common/Protocol.h"
#include "Common/Packet.h"
#include "Common/Logger.h"
#include "DB/RedisClient.h"
#include "DB/MySQLPool.h"
#include "Auth/TokenManager.h"
#include "Auth/LoginHandler.h"
#include "Match/MatchMaker.h"

#include <csignal>

static IOCPServer* g_Server = nullptr;

void SignalHandler(int sig)
{
    LOG_INFO("Signal %d received, shutting down...", sig);
    if (g_Server)
        g_Server->Stop();
}

int main()
{
    LOG_INFO("=== EDMatchServer Starting ===");

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    // ---------------------------------------------------------
    //  Redis
    // ---------------------------------------------------------
    RedisClient redis;
    if (!redis.Connect("127.0.0.1", 6379))
    {
        LOG_ERROR("Redis connection failed!");
        return 1;
    }

    // ---------------------------------------------------------
    //  MySQL
    // ---------------------------------------------------------
    MySQLPool mysql;
    MySQLConfig dbConfig;
    dbConfig.Host     = "127.0.0.1";
    dbConfig.Port     = 3306;
    dbConfig.User     = "root";
    dbConfig.Password = "0715";
    dbConfig.Database = "eternal_dreams";
    dbConfig.PoolSize = 4;

    if (!mysql.Init(dbConfig))
    {
        LOG_ERROR("MySQL connection failed!");
        return 1;
    }

    // ---------------------------------------------------------
    //  Auth
    // ---------------------------------------------------------
    TokenManager tokenMgr(redis);
    LoginHandler loginHandler(mysql, tokenMgr);

    // ---------------------------------------------------------
    //  IOCP Server
    // ---------------------------------------------------------
    IOCPServer server;
    g_Server = &server;

    // ---------------------------------------------------------
    //  MatchMaker
    // ---------------------------------------------------------
    MatchMaker matchMaker(server, redis, tokenMgr, mysql);

    // Register packet handlers
    loginHandler.Register(*server.GetPacketHandler());
    matchMaker.Register(*server.GetPacketHandler());

    server.SetOnSessionConnected([](std::shared_ptr<Session> session)
    {
        LOG_INFO("Session[%llu] Connected", session->GetId());
    });

    server.SetOnSessionDisconnected([&tokenMgr, &matchMaker](std::shared_ptr<Session> session)
    {
        // Remove from matchmaking queue on disconnect
        matchMaker.OnSessionDisconnected(session->GetId());

        // Revoke token on disconnect
        if (!session->GetAuthToken().empty())
        {
            tokenMgr.RevokeToken(session->GetAuthToken());
        }
        LOG_INFO("Session[%llu] Disconnected", session->GetId());
    });

    constexpr uint16_t PORT = 9000;

    if (!server.Init(PORT))
    {
        LOG_ERROR("Server init failed");
        return 1;
    }

    // Start matchmaking tick thread
    matchMaker.Start();

    server.Run();

    // Cleanup
    matchMaker.Stop();
    redis.Disconnect();
    mysql.Shutdown();

    LOG_INFO("=== EDMatchServer Shutdown Complete ===");
    return 0;
}
