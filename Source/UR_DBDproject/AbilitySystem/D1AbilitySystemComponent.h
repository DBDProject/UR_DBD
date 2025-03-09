// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "D1GameplayTags.h"
#include "D1AbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& StartupAbilities);

	void ActivateAbility(FGameplayTag AbilityTag);

	TArray<FGameplayAbilitySpecHandle> SpecHandles;
};
