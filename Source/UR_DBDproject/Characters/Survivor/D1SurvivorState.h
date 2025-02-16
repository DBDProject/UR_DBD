// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "D1SurvivorState.generated.h"

class UD1SurvivorSet;
class UD1AbilitySystemComponent;
/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AD1SurvivorState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UD1AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UD1SurvivorSet> SurvivorSet;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UD1AbilitySystemComponent* GetD1AbilitySystemComponent() const;
	UD1SurvivorSet* GetD1SurvivorSet() const;
};
