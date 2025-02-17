// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/D1AttributeSet.h"
#include "D1SurvivorSet.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API UD1SurvivorSet : public UD1AttributeSet
{
	GENERATED_BODY()

public:
	UD1SurvivorSet();

public:
	ATTRIBUTE_ACCESSORS(ThisClass, RunSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, CrouchSpeed);

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData RunSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData CrouchSpeed;
};
