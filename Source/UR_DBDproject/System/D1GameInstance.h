// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "D1GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UD1GameInstance(const FObjectInitializer& ObjectInitializer);

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
