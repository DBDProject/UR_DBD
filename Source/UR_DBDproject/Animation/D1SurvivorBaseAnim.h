// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/D1CharacterBaseAnim.h"
#include "D1Define.h"
#include "D1SurvivorBaseAnim.generated.h"

class AD1SurvivorBase;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class AD1SurvivorBase> D1Survivor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHealing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOpening = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESurvivorState HealingTargetState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVaultType VaultType = EVaultType::Medium;

public:
	UFUNCTION(BlueprintCallable)
	void SetIsHealing(bool bNewState) { bIsHealing = bNewState; }

	UFUNCTION(BlueprintCallable)
	void SetIsOpening(bool bNewState) { bIsOpening = bNewState; }

	UFUNCTION(BlueprintCallable)
	void SetHealingTargetState(ESurvivorState State) { HealingTargetState = State; };

	UFUNCTION(BlueprintCallable)
	void SetVaultType(EVaultType NewVaultType) { VaultType = NewVaultType; };
};
