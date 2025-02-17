// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1CharacterBase.h"
#include "D1SurvivorBase.generated.h"

class UD1SurvivorSet;
/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorBase : public AD1CharacterBase
{
	GENERATED_BODY()
	
public:
	AD1SurvivorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	virtual void InitAbilitySystem() override;
	virtual void PossessedBy(AController* NewController) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void SmoothCameraTransition(float DeltaTime);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UD1SurvivorSet> SurvivorSet;

public:
	UD1SurvivorSet* GetSurvivoreSet() const { return SurvivorSet; }
};
