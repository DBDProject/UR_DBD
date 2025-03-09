// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "D1GameState.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1GameState : public AGameState
{
	GENERATED_BODY()
	
public:
    AD1GameState();

protected:
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // 맵에 있는 ExitGate 찾기
    void FindExitGates();

public:
    // 발전기 수리 완료 시 호출
    void UpdateGeneratorState();

protected:
    // 현재 수리된 발전기 개수
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Game State")
    int32 RepairedGenerators = 0;

    // 모든 발전기가 수리되었는지 여부
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Game State")
    bool bAllGeneratorsRepaired = false;

    // 맵에 있는 탈출구들
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameProgress")
    TArray<TObjectPtr<class AD1ExitGate>> ExitGates;

    // TODO : 남아있는 플레이어
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Game State")
    int32 RemainingSurvivors;

    // TODO : 출구 열린 후 타이머
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Game State")
    float EndgameTimer;
};
