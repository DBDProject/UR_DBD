// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameMode/D1InGameMode.h"
#include "D1Define.h"
#include "D1GameState.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneratorCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInputUnlock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGateOpend);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGateEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratorRepaired, uint8, GenerateCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivorStateChanged,
	int32, SurvivorIndex, ESurvivorState, SurvivorState);

UCLASS()
class UR_DBDPROJECT_API AD1GameState : public AGameState
{
	GENERATED_BODY()

private:
	// 맵에 있는 ExitGate 찾기
	void FindExitGates();

	void FindExitAreas();

	// 일정 시간 이후 입력 잠금 해제
	void OnInputUnlockTimer();

	// 게임 시작 시 호출
	void OnGameStartTimer();

	// 일정 시간 이후 서버장 나가기
	void OnTravelTimer();

	// 일정 시간 이후 플레이어 나가기
	void OnPlayerTravelTimer();

	// UI용 변수 바뀔 시 호출
	UFUNCTION()
	void OnRep_RepairedGenerators();

	// 발전기 수리 완료 시 호출
	UFUNCTION()
	void OnRep_GeneratorCompleted();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_SetInputLock(bool bIsLock);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_GameStart();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	AD1GameState();

	// 발전기 수리 완료 시 호출
	void UpdateGeneratorState();

	void SetSurvivorState(int32 PlayerIndex, ESurvivorState State);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_SetSurvivorState(int32 PlayerIndex, ESurvivorState State);

	void ResultSurvivorGame(int32 PlayerIndex, ESurvivorState state);
	void ResultKillerGame();

private:
	FTimerHandle InputLockTimer;
	FTimerHandle GameStartTimer;
	FTimerHandle TravelTimer; // 서버장 내보내려고 만든 타이머
	FTimerHandle SurvivorTravelTimer; // 플레이어 결과창 보내려는 타이머

	// 현재 수리해야할 발전기 개수
	UPROPERTY(ReplicatedUsing = OnRep_RepairedGenerators, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	uint8 RepairedGenerators = 5;

	// 모든 발전기가 수리되었는지 여부
	UPROPERTY(ReplicatedUsing = OnRep_GeneratorCompleted)
	bool bAllGeneratorsRepaired = false;

	UPROPERTY(Replicated)
	TArray<ESurvivorState> SurvivorStates;

	bool IsGateOpened = false;

	class UD1GameStartUI* GameStartUI;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TSubclassOf<class UD1GameStartUI> GameStartUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TSubclassOf<class UD1GameEscapeUI> GameEscapeUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TSubclassOf<class UD1GameExitUI> GameExitUIClass;

	// 발전기 수리 완료 시 UI에 연결할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGeneratorRepaired OnGeneratorRepaired;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGeneratorCompleted OnGeneratorCompleted;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGameStart OnGameStart;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnInputUnlock OnInputUnlock;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGameEnd OnGameEnd;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGateOpend OnGateOpend;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGateOpend OnGateEnded;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnSurvivorStateChanged OnSurvivorStateChanged;

	// 맵에 있는 탈출구들
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TArray<TObjectPtr<class AD1ExitGate>> ExitGates;

	// 맵에 있는 탈출 Area
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TArray<TObjectPtr<class AD1ExitArea>> ExitAreas;

	// TODO : 출구 열린 후 타이머
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "DBDListen")
	float ExitRemainingTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen")
	float GATE_EXIT_TIME = 30.f;

	// 입력 잠금 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen", Meta = (Displayername = "GameStartTime"))
	float GAME_START_TIME = 4.f;

	// 입력 잠금 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen", Meta = (Displayername = "InputUnlockTime"))
	float INPUT_UNLOCK_TIMER = 5.f;

	// 탈출 후 나갈때 까지 대기 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen", Meta = (Displayername = "EscapeExitTime"))
	float ESCAPE_EXIT_TIME = 5.f;

	// 결과 후 나갈때 까지 대기 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen", Meta = (Displayername = "NormalExitTime"))
	float NORMAL_EXIT_TIME = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen", Meta = (Displayername = "NormalExitTime"))
	float KILLER_EXIT_TIME = 5.f;
};
