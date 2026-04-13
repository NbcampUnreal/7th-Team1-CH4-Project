// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Packet protocol shared with EDMatchServer (IOCP).
 * Packet layout: [Length:4][OpCode:2][Body:JSON]
 */
namespace EDNet
{
	constexpr uint32 HEADER_SIZE = 6; // 4(Length) + 2(OpCode)

	// Client -> MatchServer
	constexpr uint16 C2S_LOGIN_REQ         = 0x0001;
	constexpr uint16 C2S_MATCH_QUEUE_REQ   = 0x0010;
	constexpr uint16 C2S_MATCH_CANCEL      = 0x0040;
	constexpr uint16 C2S_LOBBY_READY       = 0x0020;
	constexpr uint16 C2S_LOBBY_TEAM_CHANGE = 0x0021;

	// MatchServer -> Client
	constexpr uint16 S2C_LOGIN_RES         = 0x0002;
	constexpr uint16 S2C_MATCH_QUEUE_RES   = 0x0011;
	constexpr uint16 S2C_MATCH_FOUND       = 0x0012;
	constexpr uint16 S2C_LOBBY_STATE       = 0x0022;
	constexpr uint16 S2C_GAME_START        = 0x0030;
}
