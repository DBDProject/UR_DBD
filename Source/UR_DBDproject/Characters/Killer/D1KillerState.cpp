// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerState.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"

AD1KillerState::AD1KillerState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UD1AbilitySystemComponent>("AbilitySystemComponent");
	KillerSet = CreateDefaultSubobject<UD1KillerSet>("KillerSet");
}

UAbilitySystemComponent* AD1KillerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UD1AbilitySystemComponent* AD1KillerState::GetD1AbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UD1KillerSet* AD1KillerState::GetD1SurvivorSet() const
{
	return KillerSet;
}
