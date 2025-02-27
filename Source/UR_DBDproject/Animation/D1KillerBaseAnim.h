// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/D1CharacterBaseAnim.h"
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
	
protected:
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

};
