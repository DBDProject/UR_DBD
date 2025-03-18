/*
	Author : 변한빛
	Last Update : 2025-03-14
	Description : 소켓을 활용하는 레벨을 위한 게임모드 클래스
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "System/D1GameInstance.h"
#include "D1NetGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncLoadMapStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncLoadMapEnd);


UCLASS()
class UR_DBDPROJECT_API AD1NetGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	UDBDNetManager* m_pNetManager;
	FTimerHandle m_gameTimerHandle;

	FString m_asyncLoadMapName;

	bool bIsAsyncLoading = false;
	bool bIsConnecting = false;

protected:
	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnAsyncLoadMapStart OnAsyncLoadMapStart;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnAsyncLoadMapEnd OnAsyncLoadMapEnd;

private:
	UFUNCTION()
	void OnLevelLoadComplete();

	void UpdateGameTime();

	// 상대경로 -> 절대경로 Game/ 기준
	FString ConvertRelativeMapPath(const FString& RelativePath);

protected:
	UFUNCTION(BlueprintCallable, Category = "DBDListen")
	void LoadAsyncGameMap(const FString& LevelName);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
