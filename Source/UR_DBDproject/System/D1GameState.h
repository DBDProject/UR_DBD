// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "D1GameState.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneratorCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratorRepaired, uint8, GenerateCount);

UCLASS()
class UR_DBDPROJECT_API AD1GameState : public AGameState
{
	GENERATED_BODY()

private:
	// 맵에 있는 ExitGate 찾기
	void FindExitGates();

	// UI용 변수 바뀔 시 호출
	UFUNCTION()
	void OnRep_RepairedGenerators();

	// 발전기 수리 완료 시 호출
	UFUNCTION()
	void OnRep_GeneratorCompleted();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	AD1GameState();

	// 발전기 수리 완료 시 호출
	void UpdateGeneratorState();

protected:
	// 발전기 수리 완료 시 UI에 연결할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGeneratorRepaired OnGeneratorRepaired;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnGeneratorCompleted OnGeneratorCompleted;

	// 현재 수리된 발전기 개수
	UPROPERTY(ReplicatedUsing = OnRep_RepairedGenerators, BlueprintReadWrite, Category = "DBDListen")
	uint8 RepairedGenerators = 5;

	// 모든 발전기가 수리되었는지 여부
	UPROPERTY(ReplicatedUsing = OnRep_GeneratorCompleted, BlueprintReadWrite, Category = "DBDListen")
	bool bAllGeneratorsRepaired = false;

	// 맵에 있는 탈출구들
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DBDListen")
	TArray<TObjectPtr<class AD1ExitGate>> ExitGates;

	// TODO : 남아있는 플레이어
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "DBDListen")
	int32 RemainingSurvivors;

	// TODO : 출구 열린 후 타이머
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "DBDListen")
	float EndgameTimer;
};
