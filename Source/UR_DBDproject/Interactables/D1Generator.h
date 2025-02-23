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
    void StartRepair(class AD1SurvivorBase* Player, EGeneratorInteractionPosition Position);

    // 수리 종료
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StopRepair(class AD1SurvivorBase* Player);

protected:
    // 오버랩 이벤트 처리 함수
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 스킬 체크 대성공
    UFUNCTION()
    void OnSkillCheckSuccess();

    // 스킬 체크 실패
    UFUNCTION()
    void OnSkillCheckFail();

    // 모든 플레이어의 수리를 중단
    UFUNCTION()
    void StopRepairAll();

    // 수리 가능 상태로 복구
    UFUNCTION()
    void EnableRepair();

    // 수리 완료
    UFUNCTION()
    void CompleteRepair();

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

    // 애님 인스턴스 캐싱
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<class UD1GeneratorAnim> CachedAnimInstance;

    // 발전기 수리중인지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generator")
    bool bIsRepairing = false;

    // 스킬 체크 실패 시 3초간 수리 불가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    bool bIsRepairBlocked = false;

    // 수리 진행도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    float RepairProgress = 0.f;

    // 수리 하고 있는 플레이어
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    TArray<TObjectPtr<class AD1SurvivorBase>> RepairingPlayers;

    // 수리 차단 해제 타이머
    FTimerHandle RepairBlockTimer;
};
