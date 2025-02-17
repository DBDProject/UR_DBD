// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/D1CharacterBaseAnim.h"
#include "D1SurvivorBaseAnim.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API UD1SurvivorBaseAnim : public UD1CharacterBaseAnim
{
	GENERATED_BODY()

public:
	UD1SurvivorBaseAnim(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AD1SurvivorBase> D1Survivor;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCrouching = false;
};
