// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "D1Define.h"
#include "D1GeneratorAnim.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GeneratorAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UD1GeneratorAnim(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGeneratorInteractionPosition InteractionPosition = EGeneratorInteractionPosition::None;

public:
	UFUNCTION(BlueprintCallable)
	EGeneratorInteractionPosition GetInteractionPosition() const { return InteractionPosition; }

	UFUNCTION(BlueprintCallable)
	void SetInteractionPosition(EGeneratorInteractionPosition NewPosition) { InteractionPosition = NewPosition; };
};
