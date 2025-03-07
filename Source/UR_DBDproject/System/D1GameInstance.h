// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DBDNetManager.h"
#include "D1GameInstance.generated.h"

/**
 *
 */

UCLASS()
class UR_DBDPROJECT_API UD1GameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBDNet")
	UDBDNetManager* m_pNetManager;

public:
	UD1GameInstance(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "GetNetManager"))
	UDBDNetManager* GetNetManager();

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
