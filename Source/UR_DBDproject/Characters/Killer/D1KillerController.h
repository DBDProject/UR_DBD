// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1Define.h"
#include "GameplayTagContainer.h"

#include "D1KillerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1KillerController : public APlayerController
{
	GENERATED_BODY()

public:
	AD1KillerController(const FObjectInitializer& ObjectInitializer);

	virtual void HandleGameplayEvent(FGameplayTag EventTag);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AD1KillerBase> D1Killer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UD1KillerSet> KillerSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class AD1SurvivorBase> CarriedSurvivor;

	/*

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Dracula_In_Wolf_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Dracula_Out_Wolf_Montage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Dracula_In_Bat_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Dracula_Out_Bat_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Bat_In_Wolf_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Bat_Out_Wolf_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Bat_In_Dracula_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Bat_Out_Dracula_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Wolf_In_Dracula_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Wolf_Out_Dracula_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Wolf_In_Bat_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KDH_Animation")
	UAnimMontage* Wolf_Out_Bat_Montage;*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> TPVAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> FPVAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> WolfAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> BatAnimInstance;

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);
	void Input_Attack1(const FInputActionValue& InputValue);
	void Input_Skill1(const FInputActionValue& InputValue);
	void Input_RightClick(const FInputActionValue& InputValue);

	void HandleInteraction();

	UFUNCTION()
	void LeftClick_Transform();
	UFUNCTION()
	void RightClick_Transform();

	UFUNCTION()
	void TransformToDracula();
	UFUNCTION()
	void TransformToWolf();
	UFUNCTION()
	void TransformToBat();
	UFUNCTION()
	void EndTransform();
	bool bTransform = false;

	UFUNCTION()
	void StartDamageGenerator();
	UFUNCTION()
	void EndDamageGenerator();
	UFUNCTION()
	void MoveToGeneratorPosition(EGeneratorInteractionPosition Position);

	UFUNCTION()
	void StartDestroyPallet();
	UFUNCTION()
	void EndDestroyPallet();

	UFUNCTION()
	void StartPickUpPlayer();
	UFUNCTION()
	void EndPickUpPlayer();

	UFUNCTION()
	void StartHookPlayer();
	UFUNCTION()
	void EndHookPlayer();

	UFUNCTION()
	void StartVault();
	UFUNCTION()
	void EndVault();
	void VaultUpdate();
	FTimerHandle VaultTimerHandle;
	float VaultTimeElapsed = 0.0f;

	UFUNCTION()
	void SetIgnoreInput(bool bEnable);
public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> TPV_DamageGenerator; // 발전기 3인칭 데미지 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> FPV_DamageGenerator; // 발전기 1인칭 데미지 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> TPV_DestroyPallet; // 팔레트 3인칭 파괴 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> FPV_DestroyPallet; // 팔레트 1인칭 파괴 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> TPV_PickUpSurvivor; // 생존자 픽업 3인칭 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> FPV_PickUpSurvivor; // 생존자 픽업 1인칭 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> TPV_HookSurvivor; // 생존자 훅(갈고리) 3인칭 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> FPV_HookSurvivor; // 생존자 훅(갈고리) 1인칭 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> TPV_VaultWindow; // 창문 넘기 3인칭 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> FPV_VaultWindow; // 창문 넘기 1인칭 몽타주

};