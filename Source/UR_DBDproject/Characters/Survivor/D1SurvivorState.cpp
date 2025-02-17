// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Survivor/D1SurvivorState.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"

AD1SurvivorState::AD1SurvivorState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UD1AbilitySystemComponent>("AbilitySystemComponent");
	SurvivorSet = CreateDefaultSubobject<UD1SurvivorSet>("SurvivorSet");
}

UAbilitySystemComponent* AD1SurvivorState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UD1AbilitySystemComponent* AD1SurvivorState::GetD1AbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UD1SurvivorSet* AD1SurvivorState::GetD1SurvivorSet() const
{
	return SurvivorSet;
}
