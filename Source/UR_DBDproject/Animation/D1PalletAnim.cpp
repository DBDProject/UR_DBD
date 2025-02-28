// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1PalletAnim.h"
#include "D1PalletAnim.h"

UD1PalletAnim::UD1PalletAnim(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UD1PalletAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Pallet = Cast<AD1Pallet>(GetOwningActor());

	PreviousState = EPalletState::Up;
	CurrentState = EPalletState::Up;
}

void UD1PalletAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Pallet == nullptr)
		return;
	
	CurrentState = Pallet->GetCurrentState();
}
