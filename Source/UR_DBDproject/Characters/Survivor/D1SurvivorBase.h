// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1CharacterBase.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"
#include "D1SurvivorBase.generated.h"

/**
 *
 */

UCLASS()
class UR_DBDPROJECT_API AD1SurvivorBase : public AD1CharacterBase
{
	GENERATED_BODY()

public:
	AD1SurvivorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	virtual void InitAbilitySystem() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SmoothCameraTransition(float DeltaTime);
	void UpdateHealingProgress(float DeltaTime);
	void MoveToVaultStartPosition();
	void MoveToPalletStartPosition();

	void StartRunning();
	// 콜리전 이벤트 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// 생존자 데미지 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void TakeDamageFromKiller();

	// 생존자 픽업 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void TakePickUpFromKiller(class AD1KillerBase* Killer);

	// 생존자 훅 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void OnHooked(class AD1Hook* Hook);

	// 생존자 치유 받는 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void BeingHealing(AD1SurvivorBase* Healer);
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void StopBeingHealing();
	UFUNCTION()
	void BeingHealing_Local(AD1SurvivorBase* Healer);
	UFUNCTION()
	void StopBeingHealing_Local();
	UFUNCTION(Server, Reliable)
	void Server_BeingHealing(AD1SurvivorBase* Healer);
	UFUNCTION(Server, Reliable)
	void Server_StopBeingHealing();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BeingHealing(AD1SurvivorBase* Healer);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopBeingHealing();    
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateHealingProgress(float NewProgress);

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void FinishHealing();

	// 아이템 장착 함수
	UFUNCTION(BlueprintCallable, Category = "Item")
	void EquipItem(TSubclassOf<AD1ItemBase> ItemClass);

	UFUNCTION(BlueprintCallable)
	void UseCurrentItem();

	void ResetHealingCooldown();

	// 레플리케이션
	UFUNCTION()
	void OnRep_SurvivorSet();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> HitMontage; // 히트 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> PickUpMontage; // 픽업 몽타주

protected:
	// 오버랩 감지용 박스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 생존자가 감지한 오브젝트 저장
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AActor> DetectedObject;

	// 상호작용 중인 발전기 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Generator> CurrentGenerator;

	// 상호작용 중인 창(VaultObject) 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1VaultObject> VaultTarget;

	// 상호작용 중인 팔레트 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Pallet> CurrentPallet;

	UPROPERTY(ReplicatedUsing = OnRep_SurvivorSet)
	TObjectPtr<UD1SurvivorSet> SurvivorSet;

	// 발전기 수리 중인지 여부
	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsRepairing = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	EGeneratorInteractionPosition InteractionPosition = EGeneratorInteractionPosition::None;

	// 스킬 체크 실패 시 
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bIsFail = false;

	// 생존자 상태 (건강, 부상, 기절)
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ESurvivorState CurrentState;

protected: // 치료 기능
	// 치료를 해주는 생존자
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AD1SurvivorBase> HealingSource = nullptr;

	// 치료 해주고 있는지
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bIsHealing = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	ESurvivorState HealingTargetState;

	// 현재 치료 진행도 (0~100%)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HealingProgress = 0.f;

	// 초당 치료 속도 (16초 동안 100% 완치)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HealingRate = 100.f / 16.f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bIsBeingHealed = false;

	// 치료가 가능한 상태인지 여부
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bCanBeHealed = true;

	// 치료 불가 타이머 핸들
	FTimerHandle HealingCooldownTimer;

	// 현재 장착 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	TWeakObjectPtr<class AD1ItemBase> EquippedItem;

	// Temp (나중에 로비에서 선택하도록 바꿔야됨)
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AD1ItemBase> BP_MedkitClass;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AD1ItemBase> BP_ToolboxClass;

public:
	AActor* GetDetectedObject() const { return DetectedObject.IsValid() ? DetectedObject.Get() : nullptr; }
	AD1Generator* GetCurrentGenerator() const { return CurrentGenerator.IsValid() ? CurrentGenerator.Get() : nullptr; }
	AD1VaultObject* GetVaultTarget() const { return VaultTarget.IsValid() ? VaultTarget.Get() : nullptr; }
	AD1Pallet* GetCurrentPallet() const { return CurrentPallet.IsValid() ? CurrentPallet.Get() : nullptr; }
	UD1SurvivorSet* GetSurvivoreSet() const { return SurvivorSet; }
	ESurvivorState GetSurvivorState() const { return CurrentState; }

	void SetSurvivorState(ESurvivorState state) { CurrentState = state; }

	void SetIsFail(bool state) { bIsFail = state; }
	void SetIsReparing(bool state) { bIsRepairing = state; }
	void SetIsHealing(bool bNewState) { bIsHealing = bNewState; }
	void SetHealingTargetState(ESurvivorState State) { HealingTargetState = State; };
	bool GetCanBeHealed() { return bCanBeHealed; }

	EGeneratorInteractionPosition GetInteractionPosition() { return InteractionPosition; }
	void SetInteractionPosition(EGeneratorInteractionPosition NewPosition) { InteractionPosition = NewPosition; };

};
