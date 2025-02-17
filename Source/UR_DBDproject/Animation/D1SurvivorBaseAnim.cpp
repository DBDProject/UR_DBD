// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1SurvivorBaseAnim.h"
#include "Characters/D1CharacterBase.h"
#include "D1SurvivorBaseAnim.h"

UD1SurvivorBaseAnim::UD1SurvivorBaseAnim(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UD1SurvivorBaseAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UD1SurvivorBaseAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	Character = Cast<AD1CharacterBase>(TryGetPawnOwner());

	if (Character == nullptr)
		return;

	if (MovementComponent == nullptr)
		return;

	bIsCrouching = Character->bIsCrouched;
}
