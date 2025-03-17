// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-17
	Description : DBD 패킷 처리 담당 클래스
*/

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "CoreMinimal.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "DBDProtocol.h"
#include "UObject/NoExportTypes.h"
#include "Packet.pb.h"
#include "DBDPacketProcessor.generated.h"
#pragma comment(lib, "ws2_32.lib")

/**
 *
 */

USTRUCT(BlueprintType)
struct FMatchInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	uint8 maxPlayer;

	UPROPERTY(BlueprintReadOnly)
	uint8 isServer;

	UPROPERTY(BlueprintReadOnly)
	FString killerIP;

	UPROPERTY(BlueprintReadOnly)
	uint8 killerCharacterType;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> survivorIPs;

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> survivorCharacterTypes;
};

// Delegate declaration for network events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChatDelegate, const FString&, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchReady, const FMatchInfo&, matchInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSurvivorMatchACK);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKillerMatchACK);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMatchCancelACK);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMatchAbandoned);

UCLASS()
class DBDNETCORE_API UDBDPacketProcessor : public UObject
{
	GENERATED_BODY()
private:
	TMap<HPACKET_TYPE,
		TFunction<void(const TSharedPtr<HPACKET>&)>>
		m_callback;

	class UDBDNetManager* m_pNetManager;

protected:
	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FChatDelegate OnChatMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FSurvivorMatchACK OnSurvivorMatchACK;

	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FKillerMatchACK OnKillerMatchACK;

	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FMatchCancelACK OnMatchCancelACK;

	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FMatchReady OnMatchReady;

	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FMatchAbandoned OnMatchAbandoned;

private:
	void ProcessChatMsg(const TSharedPtr<HPACKET>& packet);
	void ProcessKillerMatchACK(const TSharedPtr<HPACKET>& packet);
	void ProcessSurvivorMatchACK(const TSharedPtr<HPACKET>& packet);
	void ProcessMatchCancelACK(const TSharedPtr<HPACKET>& packet);
	void ProcessMatchReady(const TSharedPtr<HPACKET>& packet);
	void ProcessMatchAbandoned(const TSharedPtr<HPACKET>& packet);

public:
	void Init(class UDBDNetManager* netManager);
	void Release();

	void Process(const TSharedPtr<HPACKET>& packet);

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	void SendSurvivorMatchRequest(uint8 characterType);

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	void SendKillerMatchRequest(uint8 characterType);

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	void SendChatMessage(const FString& message);

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	void SendMatchCancel();

	template <class T>
	static bool SerializePacket(const HPACKET_TYPE packetType, const T& inSerializedData,
		HPACKET& outPacket);
	template <class T>
	static bool DeserializePacket(const HPACKET& inPacket, T& outDeserializedData);
};

template <class T>
inline bool UDBDPacketProcessor::SerializePacket(const HPACKET_TYPE packetType, const T& inSerializedData,
	HPACKET& outPacket)
{
	std::string serialized(inSerializedData.ByteSizeLong(), '\0');
	if (!inSerializedData.SerializeToString(&serialized))
	{
		UE_LOG(LogClass, Error, TEXT("Failed to serialize packet"));
		return false;
	}

	if (serialized.size() > MAX_BUFFER_SIZE)
	{
		UE_LOG(LogClass, Error, TEXT("Data size exceeds buffer limit"));
		return false;
	}
	outPacket.ph.len = PACKET_HEADER_SIZE + static_cast<int>(serialized.size());
	outPacket.ph.type = packetType;
	memcpy(outPacket.msg, serialized.c_str(), serialized.size());
	return true;
}

template <class T>
inline bool UDBDPacketProcessor::DeserializePacket(const HPACKET& inPacket, T& outDeserializedData)
{
	int packetSize = inPacket.ph.len - PACKET_HEADER_SIZE;

	if (!outDeserializedData.ParseFromArray(inPacket.msg, packetSize))
	{
		UE_LOG(LogClass, Error, TEXT("Failed to deserialize packet"));
		return false;
	}

	UE_LOG(LogClass, Log, TEXT("Deserialized packet"));
	return true;
}

