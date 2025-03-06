/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : DBD 패킷 정의
*/
#pragma once

#include "CoreMinimal.h"
#include "DBDProtocol.h"
#include "Packet.pb.h"

class DBDNETCORE_API DBDPacket
{
private:
	FCriticalSection m_cs;

	char	m_recvBuffer[MAX_BUFFER_SIZE];
	int		m_readPos;
	TQueue<TSharedPtr<HPACKET>> m_packetQueue;

private:
	void	Reset();

public:
	DBDPacket();
	virtual ~DBDPacket();

	void ProcessPacket(class UDBDNetManager*);
	void Put(char* recvBuffer, int recvSize);

	template <class T>
	static bool SerializePacket(const HPACKET_TYPE packetType, const T& inSerializedData,
		HPACKET& outPacket);
	template <class T>
	static bool DeserializePacket(const HPACKET& inPacket, T& outDeserializedData);

};

template <class T>
inline bool DBDPacket::SerializePacket(const HPACKET_TYPE packetType, const T& inSerializedData,
	HPACKET& outPacket)
{
	std::string serialized(inSerializedData.ByteSizeLong(), '\0');
	if (!inSerializedData.SerializeToString(&serialized))
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Failed to serialize packet"));
		return false;
	}

	if (serialized.size() > MAX_BUFFER_SIZE)
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Data size exceeds buffer limit"));
		return false;
	}
	outPacket.ph.len = PACKET_HEADER_SIZE + static_cast<int>(serialized.size());
	outPacket.ph.type = packetType;
	memcpy(outPacket.msg, serialized.c_str(), serialized.size());
	return true;
}

template <class T>
inline bool DBDPacket::DeserializePacket(const HPACKET& inPacket, T& outDeserializedData)
{
	int packetSize = inPacket.ph.len - PACKET_HEADER_SIZE;

	if (!outDeserializedData.ParseFromArray(inPacket.msg, packetSize))
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Failed to deserialize packet"));
		return false;
	}
	return true;
}


