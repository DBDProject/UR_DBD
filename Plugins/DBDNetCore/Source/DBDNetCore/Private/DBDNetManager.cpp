// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : DBD 네트워크 매니저 모든 소켓 네트워크 관련은 여기에 담겨져 있음
*/

#include "DBDNetManager.h"
#include "DBDNetWorker.h"

UDBDNetManager::UDBDNetManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	m_workThread = nullptr;
	m_currentThread = nullptr;
	m_bIsConnected = false;
}

void UDBDNetManager::InitWinSock()
{
	WSAData wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (ret != 0)
	{
		PrintSockError(WSAGetLastError());
	}

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET) {
		PrintSockError(WSAGetLastError());
	}

	int option = TRUE;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
}

bool UDBDNetManager::StartThread()
{
	StopThread();

	if (FPlatformProcess::SupportsMultithreading())
	{
		m_workThread = MakeShareable<DBDNetWorker>(new DBDNetWorker(this));
		m_currentThread = FRunnableThread::Create(m_workThread.Get(), TEXT("UDBDNetManager"));
		return true;
	}
	return false;
}

void UDBDNetManager::StopThread()
{
	if (m_workThread)
	{
		m_workThread->Stop();
	}
	if (m_currentThread)
	{
		m_currentThread->WaitForCompletion();
		m_currentThread->Kill();
		delete m_currentThread;
		UE_LOG(LogClass, Warning, TEXT("[DBDNet]Worker thread stopped"));
	}

	m_currentThread = nullptr;
	m_workThread = nullptr;
}

void UDBDNetManager::Init()
{
	InitWinSock();
}

void UDBDNetManager::Release()
{
	Disconnect();
	StopThread();
	WSACleanup();
}

bool UDBDNetManager::Connect(const FString& ip, const int port)
{
	// Set up the server address
	SOCKADDR_IN stServerAddr;
	ZeroMemory(&stServerAddr, sizeof(stServerAddr));
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(port);

	// Convert FString to ANSI string for inet_addr
	const char* ipAddress = TCHAR_TO_ANSI(*ip);
	stServerAddr.sin_addr.s_addr = inet_addr(ipAddress);

	// Connect to the server
	int nRet = connect(m_socket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nRet == SOCKET_ERROR) {
		PrintSockError(WSAGetLastError());
		return false;
	}

	// Start the worker thread
	if (StartThread() == false)
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Failed to start worker thread"));
		Disconnect();
		return false;
	}

	m_bIsConnected = true;
	return true;
}

void UDBDNetManager::Disconnect()
{
	if (IsConnected())
	{
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		m_bIsConnected = false;
		UE_LOG(LogClass, Warning, TEXT("[DBDNet]서버 접속 끊김"));
	}
}

bool UDBDNetManager::SendSurvivorMatchRequest()
{
	return true;
}

bool UDBDNetManager::SendKillerMatchRequest()
{
	return true;
}

bool UDBDNetManager::SendChatMessage(const FString& message)
{
	HProtocol::Chat chatMsg;
	HPACKET packet;

	chatMsg.set_msg(TCHAR_TO_UTF8(*message));
	DBDPacket::SerializePacket(HPACKET_TYPE::CHAT_MSG, chatMsg, packet);
	SendPacket(packet);

	return true;
}

void UDBDNetManager::SendPacket(const HPACKET& packet)
{
	if (IsConnected())
	{
		int nRet = send(m_socket, (const char*)&packet, packet.ph.len, 0);
		if (nRet == SOCKET_ERROR)
		{
			PrintSockError(WSAGetLastError());
			Disconnect();
			return;
		}
	}
}

void UDBDNetManager::PrintSockError(int errorCode)
{
	LPVOID lpMsgBuffer;
	// Use FormatMessageW instead of FormatMessageA to properly handle Unicode characters
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuffer,
		0,
		NULL);

	// With FormatMessageW, we get a wide string that can directly convert to FString
	FString errorMsg((WCHAR*)lpMsgBuffer);

	UE_LOG(LogClass, Error, TEXT("[DBDNet]Socket Error (%d): %s"), errorCode, *errorMsg);

	LocalFree(lpMsgBuffer);
}

bool UDBDNetManager::HasSockError(int errorCode)
{
	switch (errorCode)
	{
	case WSAEWOULDBLOCK:
		return false;

	case ERROR_IO_PENDING:
		return false;
	}

	return true;
}
