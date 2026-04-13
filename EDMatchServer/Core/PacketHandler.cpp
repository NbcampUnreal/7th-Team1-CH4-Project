#include "PacketHandler.h"
#include "Session.h"
#include "../Common/Packet.h"
#include "../Common/Logger.h"

void PacketHandler::Register(uint16_t opCode, PacketCallback callback)
{
    m_Handlers[opCode] = std::move(callback);
}

void PacketHandler::Dispatch(std::shared_ptr<Session> session, const char* packetData, uint32_t packetLen)
{
    PacketHeader header;
    if (!Packet::ParseHeader(packetData, packetLen, header))
    {
        LOG_ERROR("Session[%llu] Failed to parse packet header", session->GetId());
        return;
    }

    std::string body = Packet::ExtractBody(packetData, packetLen);

    auto it = m_Handlers.find(header.OpCode);
    if (it == m_Handlers.end())
    {
        LOG_WARN("Session[%llu] Unknown OpCode: 0x%04X", session->GetId(), header.OpCode);
        return;
    }

    LOG_DEBUG("Session[%llu] Dispatch OpCode: 0x%04X, BodyLen: %zu",
              session->GetId(), header.OpCode, body.size());

    it->second(session, body);
}
