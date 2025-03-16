// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-16
	Description : DBD 수신 받는 쓰레드 정의
*/

#include "DBDNetWorker.h"

DBDNetWorker::DBDNetWorker(UDBDNetManager* pNetManager)
{
	m_pNetManager = pNetManager;
	m_bRunning = true;
	m_readPos = 0;
}

DBDNetWorker::~DBDNetWorker()
{
	m_pNetManager = nullptr;
}

void DBDNetWorker::ReceivePacket()
{
	int recvByte = recv(m_pNetManager->m_socket, reinterpret_cast<char*>(&m_recvBuffer[m_readPos]),
		MAX_BUFFER_SIZE, 0);

	if (recvByte < 0)
	{
		if (m_pNetManager->HasSockError(WSAGetLastError()))
		{
			UE_LOG(LogClass, Error, TEXT("[DBDNet]Failed to receive data"));
			m_pNetManager->Disconnect();
			m_bRunning = false;
		}
	}
	else if (recvByte == 0)
	{
		UE_LOG(LogClass, Warning, TEXT("[DBDNet]Connection closed"));
		m_pNetManager->Disconnect();
		m_bRunning = false;
	}
	else
	{
		m_readPos += recvByte;

		if (m_readPos >= MAX_BUFFER_SIZE)
		{
			UE_LOG(LogClass, Error, TEXT("[DBDNet]Too Large Packet"));
			m_pNetManager->Disconnect();
			m_bRunning = false;
		}

		HPACKET* m_packet = (HPACKET*)m_recvBuffer;

		if (m_readPos >= PACKET_HEADER_SIZE)
		{
			if (m_packet->ph.len <= m_readPos)
			{
				TSharedPtr<HPACKET> packetData = MakeShared<HPACKET>();
				FMemory::Memzero(packetData.Get(), sizeof(HPACKET));
				FMemory::Memcpy(packetData.Get(), m_recvBuffer, static_cast<SIZE_T>(m_packet->ph.len));

				m_readPos = 0;
				m_pNetManager->AddPacket(packetData);
			}
		}
	}
}


bool DBDNetWorker::Init()
{
	return true;
}

uint32 DBDNetWorker::Run()
{
	while (m_bRunning)
	{
		if (IsValid(m_pNetManager))
		{
			ReceivePacket();
		}
		else
		{
			m_bRunning = false;
			break;
		}
		FPlatformProcess::Sleep(0.01f);
	}
	return 0;
}

void DBDNetWorker::Stop()
{
	m_bRunning = false;
}

void DBDNetWorker::Exit()
{
}
