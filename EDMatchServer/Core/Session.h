#pragma once

#include "../Common/Platform.h"

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

// ============================================================
//  IOCP Overlapped extension
// ============================================================

enum class IOType : uint8_t
{
    Accept,
    Recv,
    Send
};

struct OverlappedEx : public OVERLAPPED
{
    IOType  Type;
    WSABUF  WsaBuf;
    char    Buffer[8192];

    void Reset(IOType type)
    {
        ZeroMemory(static_cast<OVERLAPPED*>(this), sizeof(OVERLAPPED));
        Type = type;
        WsaBuf.buf = Buffer;
        WsaBuf.len = sizeof(Buffer);
    }
};

// ============================================================
//  Client session
// ============================================================

class Session
{
public:
    static constexpr uint32_t RECV_BUF_SIZE = 65536;

    explicit Session(SOCKET socket);
    ~Session();

    uint64_t GetId() const { return m_Id; }
    SOCKET   GetSocket() const { return m_Socket; }

    // Recv buffer management
    void AppendRecvData(const char* data, uint32_t len);
    bool TryExtractPacket(std::vector<char>& outPacket);

    // Async recv
    OverlappedEx* GetRecvOverlapped() { return &m_RecvOv; }
    bool PostRecv();

    // Send
    bool Send(const std::vector<char>& data);

    // Close
    void Close();
    bool IsClosed() const { return m_Closed.load(); }

    // User info (set after auth)
    void SetUserId(uint64_t userId) { m_UserId = userId; }
    uint64_t GetUserId() const { return m_UserId; }

    void SetNickname(const std::string& nick) { m_Nickname = nick; }
    const std::string& GetNickname() const { return m_Nickname; }

    void SetAuthToken(const std::string& token) { m_AuthToken = token; }
    const std::string& GetAuthToken() const { return m_AuthToken; }

    bool IsAuthenticated() const { return m_UserId != 0; }

private:
    static std::atomic<uint64_t> s_NextId;

    uint64_t            m_Id;
    SOCKET              m_Socket;
    std::atomic<bool>   m_Closed;

    std::mutex          m_RecvMutex;
    std::vector<char>   m_RecvBuffer;
    OverlappedEx        m_RecvOv;

    std::mutex          m_SendMutex;

    uint64_t            m_UserId;
    std::string         m_Nickname;
    std::string         m_AuthToken;
};
