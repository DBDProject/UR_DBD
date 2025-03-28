// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1Define.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"

#include "D1KillerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> TPVAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> FPVAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> WolfAnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KDH", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1KillerBaseAnim> BatAnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RightClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* WolfPounce_InputAction;

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);
	void Input_LeftClick(const FInputActionValue& InputValue);
	void Input_RightClick(const FInputActionValue& InputValue);
	void Input_RightClickRelease(const FInputActionValue& InputValue);
	void Input_KeyboardF(const FInputActionValue& InputValue);
	void Input_FRelease(const FInputActionValue& InputValue);
	void Input_Skill1(const FInputActionValue& InputValue);
	void Input_OnCtrlReleased(const FInputActionValue& InputValue);
	void Input_Drop(const FInputActionValue& InputValue);

	void HandleInteraction();

	UFUNCTION()
	void LeftClick_Transform();
	UFUNCTION()
	void RightClick_Transform();

	bool bIsCtrlPressed = false;
	bool bIgnoreInputLook = false;

	bool bTransform = false;

	FTimerHandle ComboDashTimer;
	void PounceTimer();
	bool CanSecondPounce = false;

public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);

	void SetIgnoreInputLook(bool bEnable) { bIgnoreInputLook = bEnable; }

	void SetbTransform(bool state) { bTransform = state; }

	void SetCanSecondPounce(bool bValue) { CanSecondPounce = bValue; }
	int32 comboIndex = 0;

};