// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-07
	Description : 소켓을 활용하는 레벨을 위한 게임모드 클래스
*/

#include "GameMode/D1NetGameMode.h"

void AD1NetGameMode::UpdateGameTime()
{
	if (!IsValid(m_pNetManager))
		return;

	if (m_pNetManager->IsConnected())
	{
		m_pNetManager->ProcessPacket();
	}
}

void AD1NetGameMode::BeginPlay()
{
	Super::BeginPlay();
	m_pNetManager = GetGameInstance<UD1GameInstance>()->GetNetManager();
	GetWorld()->GetTimerManager().SetTimer(m_gameTimerHandle
		, this, &AD1NetGameMode::UpdateGameTime, 0.2f, true);
}

void AD1NetGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(m_gameTimerHandle);
	m_pNetManager = nullptr;
}
