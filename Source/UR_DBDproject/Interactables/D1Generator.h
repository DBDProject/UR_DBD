// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1Define.h"
#include "D1Generator.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRepairSkillFail);



UENUM(BlueprintType)
enum class EGeneratorState : uint8
{
    Fix,
    Breaking,
    Idle,
};

UCLASS(Blueprintable)
class UR_DBDPROJECT_API AD1Generator : public AActor
{
	GENERATED_BODY()
public:


public:	
	// Sets default values for this actor's properties
	AD1Generator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // 플레이어의 위치에 따라 상호작용 위치 판별
    UFUNCTION(BlueprintCallable, Category = "Generator")
    EGeneratorInteractionPosition FindInteractionPosition(class AD1CharacterBase* Survivor);

    // 수리 시작
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StartRepair(class AD1SurvivorBase* Player, EGeneratorInteractionPosition Position);

    // 수리 종료
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void StopRepair(class AD1SurvivorBase* Player);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SetRepairState(AD1SurvivorBase* Player, bool bRepairing, EGeneratorInteractionPosition Position);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateRepairProgress(float NewProgress);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopRepairAll();
    // 데미지
    UFUNCTION(BlueprintCallable, Category = "Generator")
    void OnDamage();

public: // 스킬 체크
    UFUNCTION(BlueprintCallable)
    void OnSkillCheckSuccess(class AD1SurvivorBase* Player);
    UFUNCTION(BlueprintCallable)
    void OnSkillCheckFail(class AD1SurvivorBase* Player);
protected:
    UFUNCTION(NetMulticast, Reliable)
    void Multi_OnSkillCheckFail(AD1SurvivorBase* Player);

    // 모든 플레이어의 수리를 중단
    UFUNCTION()
    void StopRepairAll();

    // 수리 가능 상태로 복구
    UFUNCTION()
    void EnableRepair();

    // 수리 완료
    UFUNCTION()
    void CompleteRepair();

    // 틱 데미지
    UFUNCTION()
    void DamagePerSeconds();

    UFUNCTION(BlueprintImplementableEvent, Category = "Generator")
    void BP_CompleteRepair();

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<class UAnimMontage> G_GeneratorMontage; // 발전기 몽타주
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USceneComponent> RootScene;

    // 물리 충돌 박스
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class UBoxComponent> PhysicsCollisionBox;

    // 상호작용 범위 콜라이더
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class UBoxComponent> InteractionBox;

    // 발전기 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class USkeletalMeshComponent> GeneratorMesh;

    // 엔티티 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator")
    TObjectPtr<class USkeletalMeshComponent> EntityMesh;

    // 발전기 수리중인지
    UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Generator")
    bool bIsRepairing = false;

    // 발전기 수리완료인지
    UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Generator")
    bool bIsCompleteRepair = false;

    // 모든 플레이어 발전기 수리 불가 
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    bool bIsRepairBlockedAll = false;

    // 수리 진행도
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Generator")
    float RepairProgress = 0.f;

    // 수리 하고 있는 플레이어
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    TArray<TObjectPtr<class AD1SurvivorBase>> RepairingPlayers;

    UPROPERTY()
    TMap<EGeneratorInteractionPosition, TObjectPtr<AD1SurvivorBase>> RepairingPositions;

    UPROPERTY(Replicated, BlueprintReadOnly)
    EGeneratorInteractionPosition InteractionPosition = EGeneratorInteractionPosition::None;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    EGeneratorState CurrentState;

    // 수리 차단 해제 타이머
    FTimerHandle RepairBlockTimer;

    // 손상 횟수
    int DamageCount = 0;
    // 틱 데미지 판별
    bool bTickDamage = false;
    // 데미지 타이머 핸들
    FTimerHandle DamageTimer;

public:
    bool GetIsRepairBlocked() { return bIsRepairBlockedAll; }
    bool GetIsCompleteRepair() { return bIsCompleteRepair; }
    float GetRepairProgress() { return RepairProgress; }

    EGeneratorState GetCurrentState() { return CurrentState; }
    void SetCurrentState(EGeneratorState State) { CurrentState = State; }

    TMap<EGeneratorInteractionPosition, TObjectPtr<AD1SurvivorBase>> GetReparingPositions() { return RepairingPositions; }


    // [[[[[[ 엔티티 ]]]]]]
public:
    void ActivateEntity();
    void DeactivateEntity();

    UFUNCTION(NetMulticast, Reliable)
    void StartDissolveEffect();
    UFUNCTION(NetMulticast, Reliable)
    void StartDissolveDisappearEffect();

    bool GetEntityVisible() { return bEntityVisible; }
protected:
    void UpdateDissolve();
    void UpdateDissolveDisappear();

     UPROPERTY(Replicated)
     bool bEntityVisible;

     UPROPERTY()
     UMaterialInstanceDynamic* DynamicMat_Slot;

     UPROPERTY(Replicated)
     float CurrentDissolveValue;

     float DissolveStartTime;
     FTimerHandle DissolveTimer;
};
