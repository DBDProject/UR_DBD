// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1HookEntityAnim.h"
#include "Interactables/D1Hook.h"

UD1HookEntityAnim::UD1HookEntityAnim(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UD1HookEntityAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	D1Hook = Cast<AD1Hook>(GetOwningActor());
}

void UD1HookEntityAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}
