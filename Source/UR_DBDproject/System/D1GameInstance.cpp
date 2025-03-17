// Fill out your copyright notice in the Description page of Project Settings.

#include "System/D1GameInstance.h"
#include "D1AssetManager.h"

UD1GameInstance::UD1GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

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
}

void UD1GameInstance::SetServerInfoForDebug()
{
	for (int i = 0; i < 4; i++)
	{
		FPlayerInfo info;
		info.characterType = ECharacterType::SURVIVOR_MEG;
		info.userIP = FString(TEXT("127.0.0.1"));
		m_serverInfo.survivorInfos.Add(info);
	}

	m_serverInfo.killerInfo.characterType = ECharacterType::KILLER_DRACULA;
	m_serverInfo.killerInfo.userIP = FString(TEXT("127.0.0.1"));
	m_serverInfo.maxPlayer = 0;
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

