// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1Define.h"
#include "D1Generator.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1Generator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1Generator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
    // 상호작용 범위 콜라이더
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    class UBoxComponent* InteractionBox;

    // 발전기 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    class USkeletalMeshComponent* GeneratorMesh;

    // 플레이어의 위치에 따라 상호작용 위치 판별
    UFUNCTION(BlueprintCallable, Category = "Generator")
    EGeneratorInteractionPosition FindInteractionPosition(class AD1SurvivorBase* Survivor);

    // 발전기와 상호작용하는 플레이어 저장
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    TObjectPtr<class AD1SurvivorBase> InteractingPlayer;

    // 발전기 상태
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    bool bIsRepaired;

    // 수리 진행도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    float RepairProgress;

public:
    // 수리 시작
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StartRepair(class AD1SurvivorBase* Player);

    // 수리 종료
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StopRepair();
};
