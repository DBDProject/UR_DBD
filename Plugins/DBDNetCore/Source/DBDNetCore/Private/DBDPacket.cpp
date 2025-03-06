// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : DBD 패킷 정의
*/

#include "DBDPacket.h"
#include "DBDNetManager.h"

DBDPacket::DBDPacket()
{
	Reset();
}
DBDPacket::~DBDPacket()
{
}

void   DBDPacket::Put(char* recvBuffer, int recvSize)
{
	{
		FScopeLock lock(&m_cs);
		FMemory::Memcpy(&m_recvBuffer[m_readPos], recvBuffer, recvSize);
		m_readPos += recvSize;
	}

	if (m_readPos + recvSize >= MAX_BUFFER_SIZE)
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Too Large Packet"));
		Reset();
	}

	HPACKET* m_packet = (HPACKET*)m_recvBuffer;
	{
		FScopeLock lock(&m_cs);

		if (m_readPos >= PACKET_HEADER_SIZE)
		{
			if (m_packet->ph.len <= m_readPos)
			{
				TSharedPtr<HPACKET> AddPacket = MakeShared<HPACKET>();
				FMemory::Memzero(AddPacket.Get(), sizeof(HPACKET));
				FMemory::Memcpy(AddPacket.Get(), recvBuffer, static_cast<SIZE_T>(m_packet->ph.len));

				// 리스트에 추가
				m_packetQueue.Enqueue(AddPacket);
				Reset();
			}
		}
	}
};

void DBDPacket::ProcessPacket(UDBDNetManager* pNetManager)
{
	while (!m_packetQueue.IsEmpty())
	{
		TSharedPtr<HPACKET> packet;
		m_packetQueue.Dequeue(packet);
		m_packetQueue.Pop();

		switch (packet->ph.type)
		{
		case HPACKET_TYPE::CHAT_MSG:
			HProtocol::Chat chatMsg;
			DBDPacket::DeserializePacket(*(packet.Get()), chatMsg);

			FString fMsg = FString(UTF8_TO_TCHAR(chatMsg.msg().c_str()));
			pNetManager->OnChatMessageReceived.Broadcast(fMsg);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, fMsg);
			break;
		}
	}
}

void DBDPacket::Reset()
{
	FMemory::Memzero(m_recvBuffer, MAX_BUFFER_SIZE);
	m_readPos = 0;
}

