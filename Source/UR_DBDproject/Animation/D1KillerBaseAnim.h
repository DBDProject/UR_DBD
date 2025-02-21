// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/D1CharacterBaseAnim.h"
#include "D1KillerBaseAnim.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1KillerBaseAnim : public UD1CharacterBaseAnim
{
	GENERATED_BODY()

public:
	UD1KillerBaseAnim(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
};
