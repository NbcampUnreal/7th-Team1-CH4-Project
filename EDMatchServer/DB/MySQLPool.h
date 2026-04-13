#pragma once

#include "../Common/Platform.h"

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <cstdint>

#include <mysql/mysql.h>

// ============================================================
//  MySQL connection pool
//  - Pre-creates N connections at init
//  - GetConnection() blocks if all connections in use
//  - RAII guard auto-returns connection on scope exit
// ============================================================

struct MySQLConfig
{
    std::string Host = "127.0.0.1";
    uint16_t    Port = 3306;
    std::string User = "root";
    std::string Password = "";
    std::string Database = "eternal_dreams";
    uint32_t    PoolSize = 4;
};

// Query result: vector of rows, each row is vector of column strings
using MySQLRow = std::vector<std::string>;
using MySQLResult = std::vector<MySQLRow>;

class MySQLPool
{
public:
    MySQLPool();
    ~MySQLPool();

    bool Init(const MySQLConfig& config);
    void Shutdown();

    // Execute SELECT query, returns rows
    MySQLResult Query(const std::string& sql);

    // Execute INSERT/UPDATE/DELETE, returns affected rows (-1 on error)
    int64_t Execute(const std::string& sql);

    // Escape string for safe SQL
    std::string Escape(const std::string& input);

private:
    MYSQL* AcquireConnection();
    void ReleaseConnection(MYSQL* conn);

    MYSQL* CreateConnection();

    MySQLConfig m_Config;

    std::mutex m_Mutex;
    std::condition_variable m_Cond;
    std::queue<MYSQL*> m_Pool;
    uint32_t m_TotalCount;
};
