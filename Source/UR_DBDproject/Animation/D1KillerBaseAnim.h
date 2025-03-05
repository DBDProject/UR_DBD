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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAttacking = false;

	//공격 적중 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttackHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInTransforming = false; // 변신 스킬
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOutTransforming = false; // 변신 스킬

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLRClick = false; // 마우스 좌(false) / 우(true)클릭

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBreakingGenerator = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCarryingSurvivor = false; // 들고 있는 중인지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGeneratorInteractionPosition InteractionPosition = EGeneratorInteractionPosition::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVaultType VaultType = EVaultType::Medium;

public:
	UFUNCTION(BlueprintCallable)
	void SetIsAttacking(bool isAttack) { bIsAttacking = isAttack; }
	
	UFUNCTION(BlueprintCallable)
	bool GetIsAttacking() { return bIsAttacking; }

	UFUNCTION(BlueprintCallable)
	void SetIsInTransforming(bool bTransform) { bIsInTransforming = bTransform; }

	UFUNCTION(BlueprintCallable)
	bool GetIsInTransforming() { return bIsInTransforming; }
	
	UFUNCTION(BlueprintCallable)
	void SetIsOutTransforming(bool bTransform) { bIsOutTransforming = bTransform; }

	UFUNCTION(BlueprintCallable)
	bool GetIsOutTransforming() { return bIsOutTransforming; }

	UFUNCTION(BlueprintCallable)
	void SetAttackHit(bool bHit) { bAttackHit = bHit; }

	UFUNCTION(BlueprintCallable)
	bool GetAttackHit() { return bAttackHit; }

	UFUNCTION(BlueprintCallable)
	void SetLRClick(bool LR) { bLRClick = LR; }

	UFUNCTION(BlueprintCallable)
	bool GetLRClick() { return bLRClick; }

	UFUNCTION(BlueprintCallable)
	void SetIsBreakingGenerator(bool bBreaking) { bIsBreakingGenerator = bBreaking; }

	UFUNCTION(BlueprintCallable)
	void SetInteractionPosition(EGeneratorInteractionPosition NewPosition) { InteractionPosition = NewPosition; };

	UFUNCTION(BlueprintCallable)
	void SetIsCarryingSurvivor(bool bCarry) { bCarryingSurvivor = bCarry; }	

	UFUNCTION(BlueprintCallable)
	bool GetIsCarryingSurvivor() { return bCarryingSurvivor; }	

};
