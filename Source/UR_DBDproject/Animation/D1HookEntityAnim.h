// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "D1HookEntityAnim.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1HookEntityAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UD1HookEntityAnim(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AD1Hook> D1Hook;
};
