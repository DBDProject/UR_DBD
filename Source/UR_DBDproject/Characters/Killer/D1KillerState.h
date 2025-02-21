// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "D1KillerState.generated.h"

class UD1KillerSet;
class UD1AbilitySystemComponent;
/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1KillerState : public APlayerState
{
	GENERATED_BODY()

public:
	AD1KillerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UD1AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UD1KillerSet> KillerSet;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UD1AbilitySystemComponent* GetD1AbilitySystemComponent() const;
	UD1KillerSet* GetD1SurvivorSet() const;
};
