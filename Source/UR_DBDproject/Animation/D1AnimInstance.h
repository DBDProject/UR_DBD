// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "D1AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UD1AnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
