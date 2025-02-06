// Fill out your copyright notice in the Description page of Project Settings.

#include "System/D1GameInstance.h"
#include "D1AssetManager.h"

UD1GameInstance::UD1GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UD1GameInstance::Init()
{
	Super::Init();

	UD1AssetManager::Initialize();
}

void UD1GameInstance::Shutdown()
{
	Super::Shutdown();
}

