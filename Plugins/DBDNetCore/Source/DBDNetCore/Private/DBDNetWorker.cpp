// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : DBD 수신 받는 쓰레드 정의
*/

#include "DBDNetWorker.h"

DBDNetWorker::DBDNetWorker(UDBDNetManager* pNetManager) : m_stopTaskCounter(0)
{
	m_pNetManager = pNetManager;
}

DBDNetWorker::~DBDNetWorker()
{
	m_pNetManager = nullptr;
	m_stopTaskCounter.Decrement();
}


bool DBDNetWorker::Init()
{
	return true;
}

uint32 DBDNetWorker::Run()
{
	while (m_stopTaskCounter.GetValue() == 0)
	{
		if (IsValid(m_pNetManager))
		{
			int recvByte = recv(m_pNetManager->m_socket, reinterpret_cast<char*>(&m_recvBuffer),
				MAX_BUFFER_SIZE, 0);

			if (recvByte < 0)
			{
				if (m_pNetManager->HasSockError(WSAGetLastError()))
				{
					UE_LOG(LogClass, Error, TEXT("[DBDNet]Failed to receive data"));
					m_pNetManager->Disconnect();
					break;
				}
			}

			m_streamPacket.Put(m_recvBuffer, recvByte);
			m_streamPacket.ProcessPacket(m_pNetManager);
		}
		FPlatformProcess::Sleep(0.01f);
	}
	return 0;
}

void DBDNetWorker::Stop()
{
	m_stopTaskCounter.Increment();
}

void DBDNetWorker::Exit()
{
}
