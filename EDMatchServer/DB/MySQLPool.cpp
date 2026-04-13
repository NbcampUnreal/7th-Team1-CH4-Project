#include "MySQLPool.h"
#include "../Common/Logger.h"

MySQLPool::MySQLPool()
    : m_TotalCount(0)
{
}

MySQLPool::~MySQLPool()
{
    Shutdown();
}

bool MySQLPool::Init(const MySQLConfig& config)
{
    m_Config = config;

    for (uint32_t i = 0; i < config.PoolSize; ++i)
    {
        MYSQL* conn = CreateConnection();
        if (!conn)
        {
            LOG_ERROR("[MySQL] Failed to create connection %d/%d", i + 1, config.PoolSize);
            Shutdown();
            return false;
        }
        m_Pool.push(conn);
        m_TotalCount++;
    }

    LOG_INFO("[MySQL] Pool initialized: %d connections to %s:%d/%s",
             config.PoolSize, config.Host.c_str(), config.Port, config.Database.c_str());
    return true;
}

void MySQLPool::Shutdown()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    while (!m_Pool.empty())
    {
        MYSQL* conn = m_Pool.front();
        m_Pool.pop();
        mysql_close(conn);
    }
    m_TotalCount = 0;
    LOG_INFO("[MySQL] Pool shutdown");
}

MYSQL* MySQLPool::CreateConnection()
{
    MYSQL* conn = mysql_init(nullptr);
    if (!conn)
    {
        LOG_ERROR("[MySQL] mysql_init failed");
        return nullptr;
    }

    // Set UTF-8
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    // Reconnect on lost connection
    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

    if (!mysql_real_connect(conn,
        m_Config.Host.c_str(),
        m_Config.User.c_str(),
        m_Config.Password.c_str(),
        m_Config.Database.c_str(),
        m_Config.Port, nullptr, 0))
    {
        LOG_ERROR("[MySQL] Connect failed: %s", mysql_error(conn));
        mysql_close(conn);
        return nullptr;
    }

    return conn;
}

MYSQL* MySQLPool::AcquireConnection()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cond.wait(lock, [this] { return !m_Pool.empty(); });

    MYSQL* conn = m_Pool.front();
    m_Pool.pop();

    // Ping to check connection alive
    if (mysql_ping(conn) != 0)
    {
        LOG_WARN("[MySQL] Connection lost, reconnecting...");
        mysql_close(conn);
        conn = CreateConnection();
    }

    return conn;
}

void MySQLPool::ReleaseConnection(MYSQL* conn)
{
    if (!conn) return;

    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Pool.push(conn);
    m_Cond.notify_one();
}

MySQLResult MySQLPool::Query(const std::string& sql)
{
    MySQLResult result;

    MYSQL* conn = AcquireConnection();
    if (!conn) return result;

    if (mysql_query(conn, sql.c_str()) != 0)
    {
        LOG_ERROR("[MySQL] Query failed: %s\n  SQL: %s", mysql_error(conn), sql.c_str());
        ReleaseConnection(conn);
        return result;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res)
    {
        int numFields = mysql_num_fields(res);
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            unsigned long* lengths = mysql_fetch_lengths(res);
            MySQLRow r;
            r.reserve(numFields);
            for (int i = 0; i < numFields; ++i)
            {
                if (row[i])
                    r.emplace_back(row[i], lengths[i]);
                else
                    r.emplace_back("");
            }
            result.push_back(std::move(r));
        }

        mysql_free_result(res);
    }

    ReleaseConnection(conn);
    return result;
}

int64_t MySQLPool::Execute(const std::string& sql)
{
    MYSQL* conn = AcquireConnection();
    if (!conn) return -1;

    if (mysql_query(conn, sql.c_str()) != 0)
    {
        LOG_ERROR("[MySQL] Execute failed: %s\n  SQL: %s", mysql_error(conn), sql.c_str());
        ReleaseConnection(conn);
        return -1;
    }

    int64_t affected = (int64_t)mysql_affected_rows(conn);
    ReleaseConnection(conn);
    return affected;
}

std::string MySQLPool::Escape(const std::string& input)
{
    MYSQL* conn = AcquireConnection();
    if (!conn) return "";

    std::string escaped;
    escaped.resize(input.size() * 2 + 1);

    unsigned long len = mysql_real_escape_string(conn, &escaped[0], input.c_str(), (unsigned long)input.size());
    escaped.resize(len);

    ReleaseConnection(conn);
    return escaped;
}
