// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1SurvivorBaseAnim.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "D1SurvivorBaseAnim.h"

UD1SurvivorBaseAnim::UD1SurvivorBaseAnim(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UD1SurvivorBaseAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	D1Survivor = Cast<AD1SurvivorBase>(TryGetPawnOwner());
}

void UD1SurvivorBaseAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!D1Survivor)
	{
		APawn* OwnerPawn = TryGetPawnOwner();
		if (OwnerPawn)
		{
			D1Survivor = Cast<AD1SurvivorBase>(OwnerPawn);
		}
	}
	if (!D1Survivor) return;

	if (MovementComponent == nullptr)
		return;

	bIsCrouching = Character->bIsCrouched;
}
