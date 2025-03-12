/*
	Author : 변한빛
	Last Update : 2025-03-06
	Description : DBD 네트워크 매니저 모든 소켓 네트워크 관련은 여기에 담겨져 있음
*/

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "CoreMinimal.h"
#include "DBDProtocol.h"
#include <WinSock2.h>
#include "Packet.pb.h"
#include "DBDNetManager.generated.h"
#pragma comment(lib, "ws2_32.lib")

// Delegate declaration for network events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChatDelegate, const FString&, message);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(F, const FString&, message);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChatDelegate, const FString&, message);

UCLASS()
class DBDNETCORE_API UDBDNetManager : public UObject
{
	GENERATED_BODY()
private:
	TSharedPtr<class DBDNetWorker>	m_workThread;
	FRunnableThread* m_currentThread;
	bool m_bIsConnected;

public:
	SOCKET m_socket;
	TQueue<TSharedPtr<HPACKET>>	m_packetQueue;
	FCriticalSection			m_packetQueueLock;

	UPROPERTY(BlueprintAssignable, Category = "DBDNet")
	FChatDelegate OnChatMessageReceived;

private:
	void InitWinSock();
	bool StartThread();
	void StopThread();
	void SendPacket(const HPACKET& packet);

public:
	UDBDNetManager(const FObjectInitializer& ObjectInitializer);

	void Init();
	void Release();

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "Connect"))
	bool Connect(const FString& ip, const int port);

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "Disconnect"))
	void Disconnect();

	UFUNCTION(BlueprintPure, Category = "DBDNet", meta = (DisplayName = "IsConnected"))
	FORCEINLINE bool IsConnected() { return m_bIsConnected; }

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	bool SendSurvivorMatchRequest();

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	bool SendKillerMatchRequest();

	UFUNCTION(BluePrintCallable, Category = "DBDNet")
	bool SendChatMessage(const FString& message);

	void ProcessPacket();
	void PrintSockError(int errorCode);
	bool HasSockError(int errorCode);

	template <class T>
	static bool SerializePacket(const HPACKET_TYPE packetType, const T& inSerializedData,
		HPACKET& outPacket);
	template <class T>
	static bool DeserializePacket(const HPACKET& inPacket, T& outDeserializedData);
};

template <class T>
inline bool UDBDNetManager::SerializePacket(const HPACKET_TYPE packetType, const T& inSerializedData,
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
inline bool UDBDNetManager::DeserializePacket(const HPACKET& inPacket, T& outDeserializedData)
{
	int packetSize = inPacket.ph.len - PACKET_HEADER_SIZE;

	if (!outDeserializedData.ParseFromArray(inPacket.msg, packetSize))
	{
		UE_LOG(LogClass, Error, TEXT("[DBDNet]Failed to deserialize packet"));
		return false;
	}
	return true;
}


