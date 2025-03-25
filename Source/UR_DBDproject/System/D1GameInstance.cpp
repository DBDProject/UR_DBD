// Fill out your copyright notice in the Description page of Project Settings.

#include "System/D1GameInstance.h"
#include "D1AssetManager.h"
#include "Kismet/GameplayStatics.h"

UD1GameInstance::UD1GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UD1GameInstance::HandleNetworkFailure);
	}
}

FString UD1GameInstance::GetFailureString(ENetworkFailure::Type FailureType)
{
	switch (FailureType)
	{
	case ENetworkFailure::NetDriverAlreadyExists:      return TEXT("네트워크 드라이버가 이미 존재합니다.");
	case ENetworkFailure::NetDriverCreateFailure:      return TEXT("네트워크 드라이버 생성에 실패했습니다.");
	case ENetworkFailure::NetDriverListenFailure:      return TEXT("서버가 포트를 열 수 없습니다.");
	case ENetworkFailure::ConnectionLost:              return TEXT("서버와의 연결이 끊겼습니다.");
	case ENetworkFailure::ConnectionTimeout:           return TEXT("서버 연결 시도 시간이 초과되었습니다.");
	case ENetworkFailure::FailureReceived:             return TEXT("서버로부터 네트워크 오류가 수신되었습니다.");
	case ENetworkFailure::OutdatedClient:              return TEXT("클라이언트 버전이 서버보다 낮습니다.");
	case ENetworkFailure::OutdatedServer:              return TEXT("서버 버전이 클라이언트보다 낮습니다.");
	case ENetworkFailure::PendingConnectionFailure:    return TEXT("서버에 접속할 수 없습니다. (풀방 또는 인증 실패)");
	case ENetworkFailure::NetGuidMismatch:             return TEXT("서버와 클라이언트의 클래스/에셋 정보가 다릅니다.");
	case ENetworkFailure::NetChecksumMismatch:         return TEXT("서버와 클라이언트의 체크섬이 일치하지 않습니다.");
	default:                                           return TEXT("알 수 없는 네트워크 오류가 발생했습니다.");
	}
}

void UD1GameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	FString FailureString = GetFailureString(FailureType);
	UE_LOG(LogTemp, Error, TEXT("Network Failure: %s"), *FailureString);

	if (FailureType == ENetworkFailure::ConnectionLost || FailureType == ENetworkFailure::FailureReceived)
	{
		// 딜레이를 줘서 네트워크 관련 정리 후 OpenLevel 실행
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, [this]() {
			UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("L_Showcase1")));
			}, 0.2f, false);

		GetNetManager()->ConnectLocalServer(5000, 500);
	}
}

UDBDNetManager* UD1GameInstance::GetNetManager()
{
	if (!IsValid(m_pNetManager))
	{
		m_pNetManager = NewObject<UDBDNetManager>(this, FName("UDBDNetManager"));
		m_pNetManager->Init();
	}

	return m_pNetManager;
}

void UD1GameInstance::ClearServerInfo()
{
	m_serverInfo.survivorInfos.Empty();
	m_serverInfo.killerInfo = FPlayerInfo();
	m_serverInfo.isServer = false;
	m_serverInfo.maxPlayer = 0;
}

void UD1GameInstance::SetServerInfoForDebug()
{
	for (int i = 0; i < 4; i++)
	{
		FPlayerInfo info;
		info.characterType = ECharacterType::MEG;
		info.userIP = FString(TEXT("127.0.0.1"));
		m_serverInfo.survivorInfos.Add(info);
	}

	m_serverInfo.killerInfo.characterType = ECharacterType::DRACULA;
	m_serverInfo.killerInfo.userIP = FString(TEXT("127.0.0.1"));
	m_serverInfo.maxPlayer = 4;
}

void UD1GameInstance::SetServerInfo(const FMatchInfo& matchInfo)
{
	m_serverInfo.isServer = static_cast<bool>(matchInfo.isServer);
	m_serverInfo.maxPlayer = matchInfo.maxPlayer;
	m_serverInfo.killerInfo.characterType = static_cast<ECharacterType>(matchInfo.killerCharacterType);
	m_serverInfo.killerInfo.userIP = matchInfo.killerIP;
	for (int i = 0; i < matchInfo.survivorIPs.Num(); i++)
	{
		FPlayerInfo info;
		info.characterType = static_cast<ECharacterType>(matchInfo.survivorCharacterTypes[i]);
		info.userIP = matchInfo.survivorIPs[i];
		m_serverInfo.survivorInfos.Add(info);
	}
}

void UD1GameInstance::Init()
{
	Super::Init();

	UD1AssetManager::Initialize();

}

void UD1GameInstance::Shutdown()
{
	Super::Shutdown();

	if (IsValid(m_pNetManager))
	{
		m_pNetManager->Release();
		m_pNetManager->ConditionalBeginDestroy();
		m_pNetManager = nullptr;
	}
}

