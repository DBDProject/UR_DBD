// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DBDNetManager.h"
#include "D1Define.h"
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
	UPROPERTY(BlueprintReadWrite, Category = "DBDNet", meta = (DisplayName = "ServerInfo"))
	FServerInfo m_serverInfo;

public:
	UD1GameInstance(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "GetNetManager"))
	UDBDNetManager* GetNetManager();

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "ClearServerInfo"))
	void ClearServerInfo();

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "SetServerInfoForDebug"))
	void SetServerInfoForDebug();

	UFUNCTION(BlueprintCallable, Category = "DBDNet", meta = (DisplayName = "SetServerInfo"))
	void SetServerInfo(const FMatchInfo& matchInfo);

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
