// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/D1CharacterBaseAnim.h"
#include "D1Define.h"
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

	void UpdateYawDelta(float DeltaTime);
	
protected:
	UPROPERTY()
	float PreviousYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCarryingSurvivor = false; // 들고 있는 중인지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVaultType VaultType = EVaultType::Medium;

public:
	UFUNCTION(BlueprintCallable)
	void SetIsCarryingSurvivor(bool bCarry) { bCarryingSurvivor = bCarry; }	

	UFUNCTION(BlueprintCallable)
	bool GetIsCarryingSurvivor() { return bCarryingSurvivor; }	

};
