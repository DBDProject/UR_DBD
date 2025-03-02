// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1Define.h"
#include "D1SurvivorController.generated.h"

struct FInputActionValue;
/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorController : public APlayerController
{
	GENERATED_BODY()

public:
	AD1SurvivorController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);
	void Input_RunStart();
	void Input_RunStop();
	void Input_StartCrouch();
	void Input_StopCrouch();
	void Input_StartInteract_LeftClick();
	void Input_StopInteract_LeftClick();
	void Input_StartInteract_Space();

	void Input_StartTestInput_1();

public:
	// 발전기 수리 관련 함수
	UFUNCTION()
	void StartRepair();

	UFUNCTION()
	void StopRepair();

	// 생존자 치료
	UFUNCTION()
	void StartHealing(AD1SurvivorBase* TargetSurvivor);

	UFUNCTION()
	void StopHealing(AD1SurvivorBase* TargetSurvivor);

	UFUNCTION()
	void MoveToGeneratorPosition(EGeneratorInteractionPosition Position);

	UFUNCTION()
	void PerformVault(EVaultType VaultType);

	UFUNCTION()
	void DropPallet();

	UFUNCTION()
	void VaultPallet();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> VaultMontage; // 창 넘기기 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> PalletMontage; // 팔레트 몽타주

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AD1SurvivorBase> D1Survivor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1SurvivorBaseAnim> CachedAnimInstance;

private:
	bool bCanVaultAfterDrop = true; // DropPallet 후 VaultPallet을 막기 위한 변수
	FTimerHandle VaultCooldownTimer; // 타이머 핸들러 추가

	void EnableVaultAfterDrop(); // VaultPallet 활성화를 위한 함수


public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);
};
