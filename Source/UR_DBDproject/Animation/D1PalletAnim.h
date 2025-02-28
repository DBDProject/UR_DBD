// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Interactables/D1Pallet.h"
#include "D1PalletAnim.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1PalletAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UD1PalletAnim(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AD1Pallet> Pallet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pallet")
	EPalletState PreviousState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pallet")
	EPalletState CurrentState;
};
