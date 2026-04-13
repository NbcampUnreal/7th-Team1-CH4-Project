// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Network/EDTCPClient.h"
#include "Core/Network/EDNetProtocol.h"
#include "EternalDreams.h"
#include "Async/Async.h"
#include "Serialization/ArrayWriter.h"

FEDTCPClient::FEDTCPClient()
	: Socket(nullptr)
	, Thread(nullptr)
	, bRunning(false)
	, bConnected(false)
{
}

FEDTCPClient::~FEDTCPClient()
{
	Disconnect();
}

// ============================================================
//  Connect
// ============================================================

bool FEDTCPClient::Connect(const FString& Host, int32 Port)
{
	if (bConnected)
	{
		UE_LOG(LogEDCore, Warning, TEXT("[TCPClient] Already connected"));
		return true;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogEDCore, Error, TEXT("[TCPClient] Failed to get socket subsystem"));
		return false;
	}

	Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("EDTCPClient"), false);
	if (!Socket)
	{
		UE_LOG(LogEDCore, Error, TEXT("[TCPClient] Failed to create socket"));
		return false;
	}

	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	bool bIsValid = false;
	Addr->SetIp(*Host, bIsValid);
	if (!bIsValid)
	{
		UE_LOG(LogEDCore, Error, TEXT("[TCPClient] Invalid host: %s"), *Host);
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
		return false;
	}
	Addr->SetPort(Port);

	if (!Socket->Connect(*Addr))
	{
		UE_LOG(LogEDCore, Error, TEXT("[TCPClient] Connect failed: %s:%d"), *Host, Port);
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
		return false;
	}

	bConnected = true;
	bRunning = true;

	// Start recv thread
	Thread = FRunnableThread::Create(this, TEXT("EDTCPClient_RecvThread"), 0, TPri_Normal);

	UE_LOG(LogEDCore, Warning, TEXT("[TCPClient] Connected to %s:%d"), *Host, Port);
	return true;
}

void FEDTCPClient::Disconnect()
{
	bRunning = false;

	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}

	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}

	bConnected = false;
}

bool FEDTCPClient::IsConnected() const
{
	return bConnected;
}

// ============================================================
//  Send
// ============================================================

bool FEDTCPClient::SendPacket(uint16 OpCode, const FString& JsonBody)
{
	if (!bConnected || !Socket)
	{
		return false;
	}

	// Convert FString to UTF-8
	FTCHARToUTF8 Converter(*JsonBody);
	int32 BodyLen = Converter.Length();

	uint32 TotalLen = EDNet::HEADER_SIZE + BodyLen;

	TArray<uint8> Buf;
	Buf.SetNumUninitialized(TotalLen);

	// Write header: Length(4) + OpCode(2)
	FMemory::Memcpy(Buf.GetData(), &TotalLen, sizeof(uint32));
	FMemory::Memcpy(Buf.GetData() + sizeof(uint32), &OpCode, sizeof(uint16));

	// Write body
	if (BodyLen > 0)
	{
		FMemory::Memcpy(Buf.GetData() + EDNet::HEADER_SIZE, Converter.Get(), BodyLen);
	}

	int32 BytesSent = 0;
	if (!Socket->Send(Buf.GetData(), Buf.Num(), BytesSent))
	{
		UE_LOG(LogEDCore, Error, TEXT("[TCPClient] Send failed, OpCode=0x%04X"), OpCode);
		return false;
	}

	UE_LOG(LogEDCore, Log, TEXT("[TCPClient] Sent OpCode=0x%04X, BodyLen=%d"), OpCode, BodyLen);
	return true;
}

// ============================================================
//  FRunnable - Recv Thread
// ============================================================

bool FEDTCPClient::Init()
{
	return true;
}

uint32 FEDTCPClient::Run()
{
	while (bRunning)
	{
		if (!Socket)
		{
			break;
		}

		// Check if data available (100ms timeout)
		if (!Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(100)))
		{
			continue;
		}

		// Read header (6 bytes)
		uint8 HeaderBuf[EDNet::HEADER_SIZE];
		if (!RecvFull(HeaderBuf, EDNet::HEADER_SIZE))
		{
			break; // Disconnected
		}

		uint32 PacketLen = 0;
		uint16 OpCode = 0;
		FMemory::Memcpy(&PacketLen, HeaderBuf, sizeof(uint32));
		FMemory::Memcpy(&OpCode, HeaderBuf + sizeof(uint32), sizeof(uint16));

		// Validate
		if (PacketLen < EDNet::HEADER_SIZE || PacketLen > 65536)
		{
			UE_LOG(LogEDCore, Error, TEXT("[TCPClient] Invalid packet length: %u"), PacketLen);
			break;
		}

		// Read body
		FString JsonBody;
		uint32 BodyLen = PacketLen - EDNet::HEADER_SIZE;
		if (BodyLen > 0)
		{
			TArray<uint8> BodyBuf;
			BodyBuf.SetNumUninitialized(BodyLen + 1);

			if (!RecvFull(BodyBuf.GetData(), BodyLen))
			{
				break; // Disconnected
			}

			BodyBuf[BodyLen] = 0; // Null terminate
			FUTF8ToTCHAR Converter(reinterpret_cast<const ANSICHAR*>(BodyBuf.GetData()), BodyLen);
			JsonBody = FString(Converter.Length(), Converter.Get());
		}

		UE_LOG(LogEDCore, Log, TEXT("[TCPClient] Recv OpCode=0x%04X, BodyLen=%d"), OpCode, BodyLen);

		// Dispatch on GameThread
		uint16 RecvOpCode = OpCode;
		FString RecvBody = MoveTemp(JsonBody);

		AsyncTask(ENamedThreads::GameThread, [this, RecvOpCode, RecvBody]()
		{
			if (OnPacketReceived.IsBound())
			{
				OnPacketReceived.Execute(RecvOpCode, RecvBody);
			}
		});
	}

	// Disconnected
	bConnected = false;

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		if (OnDisconnected.IsBound())
		{
			OnDisconnected.Execute();
		}
	});

	UE_LOG(LogEDCore, Warning, TEXT("[TCPClient] Recv thread exiting"));
	return 0;
}

void FEDTCPClient::Stop()
{
	bRunning = false;
}

bool FEDTCPClient::RecvFull(uint8* Buffer, int32 Size)
{
	int32 TotalRead = 0;
	while (TotalRead < Size && bRunning)
	{
		int32 BytesRead = 0;
		if (!Socket->Recv(Buffer + TotalRead, Size - TotalRead, BytesRead))
		{
			return false;
		}
		if (BytesRead == 0)
		{
			return false; // Connection closed
		}
		TotalRead += BytesRead;
	}
	return TotalRead == Size;
}
