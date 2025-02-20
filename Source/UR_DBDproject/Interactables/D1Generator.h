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

    // 플레이어의 위치에 따라 상호작용 위치 판별
    UFUNCTION(BlueprintCallable, Category = "Generator")
    EGeneratorInteractionPosition FindInteractionPosition(class AD1SurvivorBase* Survivor);

    // 수리 시작
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StartRepair(class AD1SurvivorBase* Player);

    // 수리 종료
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StopRepair();

protected:
    // 오버랩 이벤트 처리 함수
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    // 물리 충돌 박스
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class UBoxComponent> PhysicsCollisionBox;

    // 상호작용 범위 콜라이더
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class UBoxComponent> InteractionBox;

    // 발전기 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class USkeletalMeshComponent> GeneratorMesh;

    // 발전기와 상호작용하는 플레이어 저장
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    TWeakObjectPtr<class AD1SurvivorBase> InteractingPlayer;

    // 발전기 수리중인지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator")
    bool bIsRepaired;

    // 수리 진행도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    float RepairProgress;
};
