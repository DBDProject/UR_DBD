// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-16
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

	if (!IsValid(m_packetProcessor))
	{
		m_packetProcessor = NewObject<UDBDPacketProcessor>(this, FName("UDBDPacketProcessor"));
		m_packetProcessor->Init(this);
	}
}

void UDBDNetManager::Release()
{
	Disconnect();
	StopThread();
	WSACleanup();
	m_packetProcessor->Release();

	if (m_packetProcessor)
		m_packetProcessor->ConditionalBeginDestroy();
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

bool UDBDNetManager::ConnectLocalServer(const int port)
{
	FString Subnet = GetLocalSubnet();
	UE_LOG(LogClass, Warning, TEXT("[DBDNet] Scanning subnet range: %s*"), *Subnet);

	const int NUM_THREADS = 8;  // 병렬로 검사할 스레드 수
	const int SCAN_TIMEOUT_MS = 100;  // 빠른 스캔을 위한 짧은 타임아웃

	struct FScanResult
	{
		FString IP;
		bool bFound;
		SOCKET Socket;
	};

	// 결과를 저장할 배열
	TArray<FScanResult> Results;
	Results.SetNum(NUM_THREADS);

	// 각 스레드에서 사용할 소켓 생성
	for (int i = 0; i < NUM_THREADS; ++i)
	{
		Results[i].bFound = false;
		Results[i].Socket = socket(AF_INET, SOCK_STREAM, 0);

		if (Results[i].Socket == INVALID_SOCKET) {
			PrintSockError(WSAGetLastError());
			// 실패한 소켓 정리
			for (int j = 0; j < i; ++j) {
				closesocket(Results[j].Socket);
			}
			return false;
		}

		// 논블로킹 모드로 설정
		u_long nonBlocking = 1;
		ioctlsocket(Results[i].Socket, FIONBIO, &nonBlocking);
	}

	// 각 IP 범위를 병렬로 검사
	int currentIP = 1;
	bool bFoundAny = false;

	while (currentIP <= 255 && !bFoundAny)
	{
		// 각 스레드에 검사할 IP 할당
		for (int i = 0; i < NUM_THREADS && currentIP <= 255; ++i, ++currentIP)
		{
			FString TargetIP = FString::Printf(TEXT("%s%d"), *Subnet, currentIP);
			Results[i].IP = TargetIP;

			// 소켓 주소 설정
			SOCKADDR_IN stServerAddr;
			ZeroMemory(&stServerAddr, sizeof(stServerAddr));
			stServerAddr.sin_family = AF_INET;
			stServerAddr.sin_port = htons(port);
			stServerAddr.sin_addr.s_addr = inet_addr(TCHAR_TO_ANSI(*TargetIP));

			// 연결 시도
			connect(Results[i].Socket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
		}

		// 모든 소켓 확인
		fd_set writefds, exceptfds;
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		// 모든 소켓 추가
		for (int i = 0; i < NUM_THREADS && (currentIP - NUM_THREADS + i) <= 255; ++i)
		{
			FD_SET(Results[i].Socket, &writefds);
			FD_SET(Results[i].Socket, &exceptfds);
		}

		// 타임아웃 설정
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = SCAN_TIMEOUT_MS * 1000;

		// 모든 소켓에 대해 select 호출
		select(0, NULL, &writefds, &exceptfds, &timeout);

		// 결과 확인
		for (int i = 0; i < NUM_THREADS && (currentIP - NUM_THREADS + i) <= 255; ++i)
		{
			if (FD_ISSET(Results[i].Socket, &writefds) && !FD_ISSET(Results[i].Socket, &exceptfds))
			{
				// 연결 성공
				UE_LOG(LogClass, Warning, TEXT("[DBDNet] Connected to server: %s"), *Results[i].IP);
				Results[i].bFound = true;
				bFoundAny = true;

				// 찾은 소켓 저장하고 다른 소켓은 닫기
				m_socket = Results[i].Socket;

				// 다른 소켓 정리
				for (int j = 0; j < NUM_THREADS; ++j) {
					if (j != i) {
						closesocket(Results[j].Socket);
					}
				}

				// 워커 스레드 시작
				if (StartThread())
				{
					m_bIsConnected = true;
					return true;
				}
				else
				{
					UE_LOG(LogClass, Error, TEXT("[DBDNet] Failed to start worker thread"));
					Disconnect();
					return false;
				}
			}
		}

		// 이번 배치에서 찾지 못한 경우 소켓 재설정
		if (!bFoundAny)
		{
			for (int i = 0; i < NUM_THREADS && (currentIP - NUM_THREADS + i) <= 255; ++i)
			{
				closesocket(Results[i].Socket);
				Results[i].Socket = socket(AF_INET, SOCK_STREAM, 0);

				if (Results[i].Socket == INVALID_SOCKET) {
					PrintSockError(WSAGetLastError());
					continue;
				}

				// 논블로킹 모드로 설정
				u_long nonBlocking = 1;
				ioctlsocket(Results[i].Socket, FIONBIO, &nonBlocking);
			}
		}
	}

	// 서버를 찾지 못한 경우
	if (!bFoundAny)
	{
		UE_LOG(LogClass, Warning, TEXT("[DBDNet] No servers found on subnet: %s*"), *Subnet);

		// 모든 소켓 정리
		for (int i = 0; i < NUM_THREADS; ++i) {
			closesocket(Results[i].Socket);
		}

		// 새로운 소켓 생성
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socket == INVALID_SOCKET) {
			PrintSockError(WSAGetLastError());
		}

		// TCP_NODELAY 옵션 설정
		int option = TRUE;
		setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option, sizeof(option));
	}

	return bFoundAny;
}

UDBDPacketProcessor* UDBDNetManager::GetPacketProcessor()
{
	if (!IsValid(m_packetProcessor))
	{
		m_packetProcessor = NewObject<UDBDPacketProcessor>(this, FName("UDBDPacketProcessor"));
		m_packetProcessor->Init(this);
	}

	return m_packetProcessor;
}


void UDBDNetManager::AddPacket(const TSharedPtr<HPACKET> packet)
{
	if (packet)
	{
		m_packetQueueLock.Lock();
		m_packetQueue.Enqueue(packet);
		m_packetQueueLock.Unlock();
	}
}

void UDBDNetManager::ProcessPacket()
{
	if (!m_packetQueueLock.TryLock())
		return;

	while (!m_packetQueue.IsEmpty())
	{
		TSharedPtr<HPACKET> packet;

		m_packetQueue.Dequeue(packet);
		m_packetProcessor->Process(packet);
	}

	m_packetQueueLock.Unlock();
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

FString UDBDNetManager::GetLocalSubnet()
{
	char hostName[256];
	gethostname(hostName, sizeof(hostName));

	struct addrinfo hints = { 0 }, * info, * p;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// Default value in case we don't find a valid IP
	FString SubnetIP;

	if (getaddrinfo(hostName, nullptr, &hints, &info) == 0)
	{
		// Look for a valid IPv4 address
		for (p = info; p != nullptr; p = p->ai_next)
		{
			if (p->ai_family == AF_INET) // Ensure we're only using IPv4 addresses
			{
				sockaddr_in* addr = (sockaddr_in*)p->ai_addr;
				uint32 ip = ntohl(addr->sin_addr.s_addr);
				uint32 subnet = (ip & 0xFFFFFF00);  // C class /24 subnet

				SubnetIP = FString::Printf(TEXT("%d.%d.%d."),
					(subnet >> 24) & 0xFF,
					(subnet >> 16) & 0xFF,
					(subnet >> 8) & 0xFF);
				break; // First valid IP is used then break
			}
		}

		freeaddrinfo(info);
	}

	// If we didn't find a subnet, use the default
	if (SubnetIP.IsEmpty())
	{
		SubnetIP = TEXT("192.168.0.");
	}

	return SubnetIP;
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
