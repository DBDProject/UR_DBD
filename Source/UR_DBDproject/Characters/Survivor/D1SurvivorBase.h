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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SmoothCameraTransition(float DeltaTime);
	void UpdateHealingProgress(float DeltaTime);
	void MoveToVaultStartPosition();
	void MoveToPalletStartPosition();

	// 콜리전 이벤트 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> HitMontage; // 히트 몽타주

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UD1SurvivorSet> SurvivorSet;

	// 스킬 체크 실패 시 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFail = false;

	// 생존자 상태 (건강, 부상, 기절)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ESurvivorState CurrentState;

protected: // 치료 기능
	// 현재 치료 진행도 (0~100%)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HealingProgress = 0.f;

	// 초당 치료 속도 (16초 동안 100% 완치)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HealingRate = 100.f / 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bIsBeingHealed = false;

public:
	// 생존자 데미지 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void TakeDamageFromKiller();

	// 생존자 치유 받는 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void BeingHealing(AD1SurvivorBase* Healer);

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void StopBeingHealing();

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void FinishHealing();

public:
	AActor* GetDetectedObject() const { return DetectedObject.IsValid() ? DetectedObject.Get() : nullptr; }
	AD1Generator* GetCurrentGenerator() const { return CurrentGenerator.IsValid() ? CurrentGenerator.Get() : nullptr; }
	AD1VaultObject* GetVaultTarget() const { return VaultTarget.IsValid() ? VaultTarget.Get() : nullptr; }
	AD1Pallet* GetCurrentPallet() const { return CurrentPallet.IsValid() ? CurrentPallet.Get() : nullptr; }
	UD1SurvivorSet* GetSurvivoreSet() const { return SurvivorSet; }
	ESurvivorState GetSurvivorState() const { return CurrentState; }

	void SetIsFail(bool state) { bIsFail = state; }
};
