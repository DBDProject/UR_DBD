// Fill out your copyright notice in the Description page of Project Settings.

#include "System/D1GameInstance.h"
#include "D1AssetManager.h"

UD1GameInstance::UD1GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UDBDNetManager* UD1GameInstance::GetNetManager()
{
	return m_pNetManager;
}

void UD1GameInstance::Init()
{
	Super::Init();

	UD1AssetManager::Initialize();

	if (!IsValid(m_pNetManager))
	{
		m_pNetManager = NewObject<UDBDNetManager>(this, FName("UDBDNetManager"));
		m_pNetManager->Init();
	}
}

void UD1GameInstance::Shutdown()
{
	Super::Shutdown();

	if (IsValid(m_pNetManager))
	{
		m_pNetManager->Release();
		m_pNetManager->ConditionalBeginDestroy();
	}
}

