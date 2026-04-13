#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

class Session;

using PacketCallback = std::function<void(std::shared_ptr<Session>, const std::string& jsonBody)>;

class PacketHandler
{
public:
    void Register(uint16_t opCode, PacketCallback callback);
    void Dispatch(std::shared_ptr<Session> session, const char* packetData, uint32_t packetLen);

private:
    std::unordered_map<uint16_t, PacketCallback> m_Handlers;
};
