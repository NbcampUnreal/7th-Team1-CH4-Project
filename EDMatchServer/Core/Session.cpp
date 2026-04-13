#include "Session.h"
#include "../Common/Packet.h"
#include "../Common/Logger.h"

std::atomic<uint64_t> Session::s_NextId{ 1 };

Session::Session(SOCKET socket)
    : m_Id(s_NextId.fetch_add(1))
    , m_Socket(socket)
    , m_Closed(false)
    , m_UserId(0)
{
    m_RecvBuffer.reserve(RECV_BUF_SIZE);
    m_RecvOv.Reset(IOType::Recv);
}

Session::~Session()
{
    Close();
}

void Session::AppendRecvData(const char* data, uint32_t len)
{
    std::lock_guard<std::mutex> lock(m_RecvMutex);
    m_RecvBuffer.insert(m_RecvBuffer.end(), data, data + len);
}

bool Session::TryExtractPacket(std::vector<char>& outPacket)
{
    std::lock_guard<std::mutex> lock(m_RecvMutex);

    if (m_RecvBuffer.size() < Protocol::HEADER_SIZE)
        return false;

    uint32_t packetLen = 0;
    std::memcpy(&packetLen, m_RecvBuffer.data(), sizeof(uint32_t));

    if (packetLen < Protocol::HEADER_SIZE || packetLen > RECV_BUF_SIZE)
    {
        LOG_ERROR("Session[%llu] Invalid packet length: %u", m_Id, packetLen);
        return false;
    }

    if (m_RecvBuffer.size() < packetLen)
        return false;

    outPacket.assign(m_RecvBuffer.begin(), m_RecvBuffer.begin() + packetLen);
    m_RecvBuffer.erase(m_RecvBuffer.begin(), m_RecvBuffer.begin() + packetLen);

    return true;
}

bool Session::PostRecv()
{
    if (m_Closed.load())
        return false;

    m_RecvOv.Reset(IOType::Recv);

    DWORD flags = 0;
    DWORD bytesRecv = 0;

    int result = WSARecv(m_Socket, &m_RecvOv.WsaBuf, 1, &bytesRecv, &flags, &m_RecvOv, nullptr);

    if (result == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING)
        {
            LOG_ERROR("Session[%llu] WSARecv failed: %d", m_Id, err);
            return false;
        }
    }

    return true;
}

bool Session::Send(const std::vector<char>& data)
{
    if (m_Closed.load() || data.empty())
        return false;

    std::lock_guard<std::mutex> lock(m_SendMutex);

    uint32_t totalSent = 0;
    while (totalSent < data.size())
    {
        int sent = send(m_Socket, data.data() + totalSent,
                        static_cast<int>(data.size() - totalSent), 0);
        if (sent == SOCKET_ERROR)
        {
            LOG_ERROR("Session[%llu] send failed: %d", m_Id, WSAGetLastError());
            return false;
        }
        totalSent += sent;
    }

    return true;
}

void Session::Close()
{
    bool expected = false;
    if (!m_Closed.compare_exchange_strong(expected, true))
        return;

    if (m_Socket != INVALID_SOCKET)
    {
        shutdown(m_Socket, SD_BOTH);
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
    }

    LOG_INFO("Session[%llu] Closed", m_Id);
}
