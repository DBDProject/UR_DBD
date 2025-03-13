// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameMode/D1InGameMode.h"
#include "D1GameState.generated.h"

/**
 *
 */

#define GAMESTART_PLAYER_COUNT 2

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneratorCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratorRepaired, uint8, GenerateCount);

UCLASS()
class UR_DBDPROJECT_API AD1GameState : public AGameState
{
	GENERATED_BODY()

private:
	// 맵에 있는 ExitGate 찾기
	void FindExitGates();

	// 맵에 있는 PlayerSpawner 찾기
	void FindPlayerSpawners();

	// 플레이어 위치 설정
	void SetPlayerLocation();

	// UI용 변수 바뀔 시 호출
	UFUNCTION()
	void OnRep_RepairedGenerators();

	// 발전기 수리 완료 시 호출
	UFUNCTION()
	void OnRep_GeneratorCompleted();

	UFUNCTION()
	void OnRep_ReadyPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleMatchHasStarted() override;

public:
	AD1GameState();

	// 발전기 수리 완료 시 호출
	void UpdateGeneratorState();

	// 플레이어 로딩 완료 시 호출
	UFUNCTION(Server, Reliable)
	void ReadyPlayer();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen")
	FString InGameMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen")
	bool bIsAsnycLoadMap = false;

protected:
	// 발전기 수리 완료 시 UI에 연결할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGeneratorRepaired OnGeneratorRepaired;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGeneratorCompleted OnGeneratorCompleted;

	// 현재 수리해야할 발전기 개수
	UPROPERTY(ReplicatedUsing = OnRep_RepairedGenerators, BlueprintReadWrite, Category = "DBDListen")
	uint8 RepairedGenerators = 5;

	// 모든 발전기가 수리되었는지 여부
	UPROPERTY(ReplicatedUsing = OnRep_GeneratorCompleted, BlueprintReadWrite, Category = "DBDListen")
	bool bAllGeneratorsRepaired = false;

	// 맵에 있는 탈출구들
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TArray<TObjectPtr<class AD1ExitGate>> ExitGates;

	// 맵에 있는 스포너들 
	UPROPERTY(VisibleAnywhere, Category = "DBDListen")
	TArray<AActor*> KillerSpawners;

	// 맵에 있는 스포너들 
	UPROPERTY(VisibleAnywhere, Category = "DBDListen")
	TArray<AActor*> SurvivorSpawners;

	// TODO : 남아있는 플레이어
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "DBDListen")
	int32 RemainingSurvivors;

	// TODO : 출구 열린 후 타이머
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "DBDListen")
	float EndGameTimer;


private:
	UPROPERTY(ReplicatedUsing = OnRep_ReadyPlayer)
	uint8 nReadyPlayerCount = 0;

};
