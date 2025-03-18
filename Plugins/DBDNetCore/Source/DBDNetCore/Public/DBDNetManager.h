/*
	Author : 변한빛
	Last Update : 2025-03-18
	Description : DBD 네트워크 매니저 모든 소켓 네트워크 관련은 여기에 담겨져 있음
*/

#pragma once

#include "CoreMinimal.h"
#include "DBDProtocol.h"
#include "DBDPacketProcessor.h"
#include "DBDNetManager.generated.h"

UCLASS()
class DBDNETCORE_API UDBDNetManager : public UObject
{
	GENERATED_BODY()
private:
	TSharedPtr<class DBDNetWorker>	m_workThread;

	FRunnableThread* m_currentThread;

	bool m_bIsConnected;

	TQueue<TSharedPtr<HPACKET>>	m_packetQueue;
	FCriticalSection			m_packetQueueLock;


protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBDNet")
	UDBDPacketProcessor* m_packetProcessor;

public:
	SOCKET m_socket;

private:
	void InitWinSock();
	bool StartThread();
	void StopThread();

	bool IsSameSubnet(const FString& LocalSubnet, const FString& TargetIP);
	FString GetLocalSubnet();
	FString GetLocalIP();
	TArray<FString> GetARPTable();

public:
	UDBDNetManager(const FObjectInitializer& ObjectInitializer);

	void Init();
	void Release();

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "Connect"))
	bool Connect(const FString& ip, const int port, int timeoutMs);

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "Disconnect"))
	void Disconnect();

	UFUNCTION(BlueprintPure, Category = "DBDNet", meta = (DisplayName = "IsConnected"))
	FORCEINLINE bool IsConnected() { return m_bIsConnected; }

	UFUNCTION(BlueprintCallable, Category = "DBDNet")
	bool ConnectLocalServer(const int port);

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "GetPacketProcessor"))
	UDBDPacketProcessor* GetPacketProcessor();

	void SendPacket(const HPACKET& packet);
	void AddPacket(const TSharedPtr<HPACKET> packet);
	void ProcessPacket();
	void PrintSockError(int errorCode);
	bool HasSockError(int errorCode);

};
