#pragma once

// WinSock2.h MUST come before Windows.h and any standard headers
// that might pull in Windows.h. Include this header first everywhere.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")
