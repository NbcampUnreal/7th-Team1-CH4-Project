#include "IOCPServer.h"
#include "Session.h"
#include "PacketHandler.h"
#include "../Common/Logger.h"

IOCPServer::IOCPServer()
    : m_IOCP(INVALID_HANDLE_VALUE)
    , m_ListenSocket(INVALID_SOCKET)
    , m_Port(0)
    , m_WorkerCount(0)
    , m_Running(false)
    , m_PacketHandler(std::make_unique<PacketHandler>())
{
}

IOCPServer::~IOCPServer()
{
    Stop();
}

// ============================================================
//  Init
// ============================================================

bool IOCPServer::Init(uint16_t port, uint32_t workerThreadCount)
{
    m_Port = port;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_ERROR("WSAStartup failed");
        return false;
    }

    m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_IOCP == NULL)
    {
        LOG_ERROR("CreateIoCompletionPort failed: %d", GetLastError());
        return false;
    }

    m_ListenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (m_ListenSocket == INVALID_SOCKET)
    {
        LOG_ERROR("WSASocket failed: %d", WSAGetLastError());
        return false;
    }

    int opt = 1;
    setsockopt(m_ListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(m_ListenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        LOG_ERROR("bind failed: %d", WSAGetLastError());
        return false;
    }

    if (listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        LOG_ERROR("listen failed: %d", WSAGetLastError());
        return false;
    }

    if (workerThreadCount == 0)
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        m_WorkerCount = sysInfo.dwNumberOfProcessors * 2;
    }
    else
    {
        m_WorkerCount = workerThreadCount;
    }

    LOG_INFO("IOCPServer initialized - Port: %d, Workers: %d", port, m_WorkerCount);
    return true;
}

// ============================================================
//  Run (Accept loop)
// ============================================================

void IOCPServer::Run()
{
    m_Running.store(true);

    for (uint32_t i = 0; i < m_WorkerCount; ++i)
    {
        m_WorkerThreads.emplace_back(&IOCPServer::WorkerThread, this);
    }

    LOG_INFO("IOCPServer running on port %d...", m_Port);

    while (m_Running.load())
    {
        sockaddr_in clientAddr = {};
        int addrLen = sizeof(clientAddr);

        SOCKET clientSocket = accept(m_ListenSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            if (m_Running.load())
                LOG_ERROR("accept failed: %d", WSAGetLastError());
            continue;
        }

        char ipStr[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
        LOG_INFO("Client connected: %s:%d", ipStr, ntohs(clientAddr.sin_port));

        auto session = std::make_shared<Session>(clientSocket);

        if (CreateIoCompletionPort((HANDLE)clientSocket, m_IOCP,
            (ULONG_PTR)session->GetId(), 0) == NULL)
        {
            LOG_ERROR("Failed to associate socket with IOCP: %d", GetLastError());
            session->Close();
            continue;
        }

        AddSession(session);

        if (m_OnConnected)
            m_OnConnected(session);

        if (!session->PostRecv())
        {
            LOG_ERROR("Session[%llu] PostRecv failed on accept", session->GetId());
            RemoveSession(session->GetId());
        }
    }
}

// ============================================================
//  Worker Thread
// ============================================================

void IOCPServer::WorkerThread()
{
    while (m_Running.load())
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OVERLAPPED* overlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(
            m_IOCP, &bytesTransferred, &completionKey, &overlapped, 1000);

        if (!result)
        {
            if (overlapped == nullptr)
                continue;

            uint64_t sessionId = completionKey;
            auto session = FindSession(sessionId);
            if (session)
            {
                LOG_WARN("Session[%llu] I/O error, disconnecting", sessionId);
                RemoveSession(sessionId);
            }
            continue;
        }

        if (bytesTransferred == 0 && overlapped == nullptr)
            break;

        uint64_t sessionId = completionKey;
        auto session = FindSession(sessionId);
        if (!session)
            continue;

        if (bytesTransferred == 0)
        {
            LOG_INFO("Session[%llu] Disconnected (graceful)", sessionId);
            RemoveSession(sessionId);
            continue;
        }

        OverlappedEx* ovEx = static_cast<OverlappedEx*>(overlapped);

        if (ovEx->Type == IOType::Recv)
        {
            session->AppendRecvData(ovEx->Buffer, bytesTransferred);

            std::vector<char> packetData;
            while (session->TryExtractPacket(packetData))
            {
                m_PacketHandler->Dispatch(session,
                    packetData.data(), static_cast<uint32_t>(packetData.size()));
            }

            if (!session->PostRecv())
            {
                LOG_ERROR("Session[%llu] PostRecv failed", sessionId);
                RemoveSession(sessionId);
            }
        }
    }
}

// ============================================================
//  Stop
// ============================================================

void IOCPServer::Stop()
{
    if (!m_Running.exchange(false))
        return;

    LOG_INFO("IOCPServer stopping...");

    if (m_ListenSocket != INVALID_SOCKET)
    {
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
    }

    for (uint32_t i = 0; i < m_WorkerCount; ++i)
    {
        PostQueuedCompletionStatus(m_IOCP, 0, 0, nullptr);
    }

    for (auto& t : m_WorkerThreads)
    {
        if (t.joinable())
            t.join();
    }
    m_WorkerThreads.clear();

    {
        std::lock_guard<std::mutex> lock(m_SessionMutex);
        for (auto& pair : m_Sessions)
        {
            pair.second->Close();
        }
        m_Sessions.clear();
    }

    if (m_IOCP != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_IOCP);
        m_IOCP = INVALID_HANDLE_VALUE;
    }

    WSACleanup();
    LOG_INFO("IOCPServer stopped");
}

// ============================================================
//  Session management
// ============================================================

void IOCPServer::AddSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(m_SessionMutex);
    m_Sessions[session->GetId()] = session;
    LOG_INFO("Session[%llu] Added (total: %zu)", session->GetId(), m_Sessions.size());
}

void IOCPServer::RemoveSession(uint64_t sessionId)
{
    std::shared_ptr<Session> session;
    {
        std::lock_guard<std::mutex> lock(m_SessionMutex);
        auto it = m_Sessions.find(sessionId);
        if (it == m_Sessions.end())
            return;
        session = it->second;
        m_Sessions.erase(it);
    }

    session->Close();

    if (m_OnDisconnected)
        m_OnDisconnected(session);

    LOG_INFO("Session[%llu] Removed (total: %zu)", sessionId, m_Sessions.size());
}

std::shared_ptr<Session> IOCPServer::FindSession(uint64_t sessionId)
{
    std::lock_guard<std::mutex> lock(m_SessionMutex);
    auto it = m_Sessions.find(sessionId);
    if (it != m_Sessions.end())
        return it->second;
    return nullptr;
}

void IOCPServer::BroadcastToAll(const std::vector<char>& data)
{
    std::lock_guard<std::mutex> lock(m_SessionMutex);
    for (auto& pair : m_Sessions)
    {
        pair.second->Send(data);
    }
}
