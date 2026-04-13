// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

/**
 * FEDTCPClient
 *
 * FRunnable-based TCP client for communicating with EDMatchServer.
 * Runs a recv loop on a separate thread, dispatches received packets via delegate.
 */

DECLARE_DELEGATE_TwoParams(FOnPacketReceived, uint16 /*OpCode*/, const FString& /*JsonBody*/);
DECLARE_DELEGATE(FOnDisconnected);

class ETERNALDREAMS_API FEDTCPClient : public FRunnable
{
public:
	FEDTCPClient();
	virtual ~FEDTCPClient();

	// Connect to EDMatchServer
	bool Connect(const FString& Host, int32 Port);
	void Disconnect();
	bool IsConnected() const;

	// Send packet: OpCode + JSON body
	bool SendPacket(uint16 OpCode, const FString& JsonBody);

	// Delegates
	FOnPacketReceived OnPacketReceived;
	FOnDisconnected OnDisconnected;

	// --- FRunnable ---
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	bool RecvFull(uint8* Buffer, int32 Size);

	FSocket* Socket;
	FRunnableThread* Thread;

	FThreadSafeBool bRunning;
	FThreadSafeBool bConnected;

	// Recv buffer
	TArray<uint8> RecvBuffer;
};
