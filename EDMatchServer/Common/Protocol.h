#pragma once

#include <cstdint>

// ============================================================
//  OpCode 정의 - 클라이언트 <-> IOCP 서버 <-> 데디서버
// ============================================================

namespace Protocol
{
    // --- 패킷 헤더 ---
    constexpr uint16_t HEADER_SIZE = 6; // 4(길이) + 2(OpCode)

    // --- 클라이언트 <-> 서버 ---
    constexpr uint16_t C2S_LOGIN_REQ        = 0x0001;
    constexpr uint16_t S2C_LOGIN_RES        = 0x0002;
    constexpr uint16_t C2S_REGISTER_REQ     = 0x0003;
    constexpr uint16_t S2C_REGISTER_RES     = 0x0004;

    constexpr uint16_t C2S_MATCH_QUEUE_REQ  = 0x0010;
    constexpr uint16_t S2C_MATCH_QUEUE_RES  = 0x0011;
    constexpr uint16_t S2C_MATCH_FOUND      = 0x0012;
    constexpr uint16_t C2S_MATCH_CANCEL     = 0x0040;

    constexpr uint16_t C2S_LOBBY_READY      = 0x0020;
    constexpr uint16_t C2S_LOBBY_TEAM_CHANGE = 0x0021;
    constexpr uint16_t S2C_LOBBY_STATE      = 0x0022;

    constexpr uint16_t S2C_GAME_START       = 0x0030;

    // --- 데디서버 <-> 서버 ---
    constexpr uint16_t D2S_DEDI_REGISTER    = 0x1001;
    constexpr uint16_t S2D_DEDI_ASSIGN      = 0x1002;
    constexpr uint16_t D2S_DEDI_HEARTBEAT   = 0x1003;
    constexpr uint16_t D2S_DEDI_MATCH_RESULT = 0x1004;
}
