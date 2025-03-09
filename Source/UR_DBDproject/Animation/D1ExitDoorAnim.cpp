// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1ExitDoorAnim.h"
#include "Interactables/D1ExitGate.h"

UD1ExitDoorAnim::UD1ExitDoorAnim(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UD1ExitDoorAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ExitGate = Cast<AD1ExitGate>(GetOwningActor());
}

void UD1ExitDoorAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}
