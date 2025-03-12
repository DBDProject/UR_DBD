// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1CharacterBase.h"
#include "D1KillerBase.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1KillerBase : public AD1CharacterBase
{
	GENERATED_BODY()

public:
	AD1KillerBase();

	virtual void HandleGameplayEvent(FGameplayTag EventTag) override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitAbilitySystem() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> WolfCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> BatCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> BatSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> CharacterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> FPVMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> WolfMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> BatMesh;

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

	// 상호작용 중인 훅 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Hook> CurrentHook;
	
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AD1SurvivorBase> DetectedSurvivor;
	
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AD1SurvivorBase> DetectedCrawlSurvivor;

	bool bSurvivorHit = false;

private:
	UFUNCTION()
	void OnOverlapObjectBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapObjectEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnOverlapPlayerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapPlayerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	UFUNCTION(BlueprintCallable, Category = KDH_Camera)
	void SwitchCamera(EDraculaTransformationState NewState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	UBoxComponent* AttackCollision;

	TObjectPtr<USkeletalMeshComponent> GetCharacterMesh() const { return CharacterMesh; }
	TObjectPtr<USkeletalMeshComponent> GetFPVMesh() const { return FPVMesh; }
	TObjectPtr<USkeletalMeshComponent> GetWolfMesh() const { return WolfMesh; }
	TObjectPtr<USkeletalMeshComponent> GetBatMesh() const { return BatMesh; }

	TObjectPtr<UCameraComponent> GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	TObjectPtr<UCameraComponent> GetWolfCameraComponent() const { return WolfCameraComponent; }
	TObjectPtr<UCameraComponent> GetBatCameraComponent() const { return BatCameraComponent; }


	AActor* GetDetectedObject() const { return DetectedObject.IsValid() ? DetectedObject.Get() : nullptr; }
	AD1Generator* GetCurrentGenerator() const { return CurrentGenerator.IsValid() ? CurrentGenerator.Get() : nullptr; }
	AD1VaultObject* GetVaultTarget() const { return VaultTarget.IsValid() ? VaultTarget.Get() : nullptr; }
	AD1Pallet* GetCurrentPallet() const { return CurrentPallet.IsValid() ? CurrentPallet.Get() : nullptr; }
	AD1Hook* GetCurrentHook() const { return CurrentHook.IsValid() ? CurrentHook.Get() : nullptr; }
	AD1SurvivorBase* GetDetectedSurvivor() const { return DetectedSurvivor.IsValid() ? DetectedSurvivor.Get() : nullptr; }
	AD1SurvivorBase* GetDetectedCrawlSurvivor() const { return DetectedCrawlSurvivor.IsValid() ? DetectedCrawlSurvivor.Get() : nullptr; }

	void ActivateAbility(FGameplayTag AbilityTag);
	UD1AbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }

	bool GetbSurvivorHit() { return bSurvivorHit; }
};
