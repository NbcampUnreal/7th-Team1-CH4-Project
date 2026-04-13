#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "Protocol.h"

// ============================================================
//  Packet structure
//  [PacketLength: 4 bytes] [OpCode: 2 bytes] [Body: variable JSON]
//  PacketLength = total size including header
// ============================================================

struct PacketHeader
{
    uint32_t Length;
    uint16_t OpCode;
};

class Packet
{
public:
    static bool ParseHeader(const char* data, uint32_t dataLen, PacketHeader& outHeader)
    {
        if (dataLen < Protocol::HEADER_SIZE)
            return false;

        std::memcpy(&outHeader.Length, data, sizeof(uint32_t));
        std::memcpy(&outHeader.OpCode, data + sizeof(uint32_t), sizeof(uint16_t));
        return true;
    }

    static std::string ExtractBody(const char* data, uint32_t packetLength)
    {
        if (packetLength <= Protocol::HEADER_SIZE)
            return std::string();

        uint32_t bodyLen = packetLength - Protocol::HEADER_SIZE;
        return std::string(data + Protocol::HEADER_SIZE, bodyLen);
    }

    static std::vector<char> Build(uint16_t opCode, const std::string& jsonBody)
    {
        uint32_t totalLen = Protocol::HEADER_SIZE + static_cast<uint32_t>(jsonBody.size());
        std::vector<char> buf(totalLen);

        std::memcpy(buf.data(), &totalLen, sizeof(uint32_t));
        std::memcpy(buf.data() + sizeof(uint32_t), &opCode, sizeof(uint16_t));

        if (!jsonBody.empty())
        {
            std::memcpy(buf.data() + Protocol::HEADER_SIZE, jsonBody.data(), jsonBody.size());
        }

        return buf;
    }
};
