#pragma once

#include "../Common/Platform.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <functional>
#include <atomic>

class Session;
class PacketHandler;

// ============================================================
//  IOCP TCP Server
//
//  1. Listen socket accepts clients
//  2. Associates client sockets with IOCP
//  3. Worker threads handle I/O completion (Recv -> parse -> dispatch)
// ============================================================

class IOCPServer
{
public:
    using SessionCallback = std::function<void(std::shared_ptr<Session>)>;

    IOCPServer();
    ~IOCPServer();

    bool Init(uint16_t port, uint32_t workerThreadCount = 0);
    void Run();
    void Stop();

    PacketHandler* GetPacketHandler() const { return m_PacketHandler.get(); }

    void SetOnSessionConnected(SessionCallback cb)     { m_OnConnected = std::move(cb); }
    void SetOnSessionDisconnected(SessionCallback cb)   { m_OnDisconnected = std::move(cb); }

    std::shared_ptr<Session> FindSession(uint64_t sessionId);
    void BroadcastToAll(const std::vector<char>& data);

private:
    void WorkerThread();

    void AddSession(std::shared_ptr<Session> session);
    void RemoveSession(uint64_t sessionId);

    HANDLE   m_IOCP;
    SOCKET   m_ListenSocket;
    uint16_t m_Port;

    std::vector<std::thread> m_WorkerThreads;
    uint32_t m_WorkerCount;

    std::mutex m_SessionMutex;
    std::unordered_map<uint64_t, std::shared_ptr<Session>> m_Sessions;

    std::unique_ptr<PacketHandler> m_PacketHandler;

    SessionCallback m_OnConnected;
    SessionCallback m_OnDisconnected;

    std::atomic<bool> m_Running;
};
