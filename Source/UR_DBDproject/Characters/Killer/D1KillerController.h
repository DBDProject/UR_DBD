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

public:
	/*UFUNCTION(BlueprintCallable, Category = "KDH_Camera")
	void SwitchCameraToState(ECreatureState NewState, float BlendTime = 1.0f);*/

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);

	UFUNCTION()
	void Input_Attack1(const FInputActionValue& InputValue);

	void Input_Skill1(const FInputActionValue& InputValue);
	void Input_RightClick(const FInputActionValue& InputValue);

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
	void SetIgnoreInput(bool bEnable);
public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> TPV_DamageGenerator; // 발전기 데미지 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> FPV_DamageGenerator; // 발전기 데미지 몽타주

};