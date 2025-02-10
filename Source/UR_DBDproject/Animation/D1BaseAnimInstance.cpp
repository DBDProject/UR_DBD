// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1BaseAnimInstance.h"
#include "Characters/D1CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

UD1BaseAnimInstance::UD1BaseAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UD1BaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AD1CharacterBase>(TryGetPawnOwner());

	if (Character)
		MovementComponent = Character->GetCharacterMovement();
}

void UD1BaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr)
		return;

	if (MovementComponent == nullptr)
		return;

	Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();

	bShouldMove = (GroundSpeed > 3.f && MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector);
	bIsFalling = MovementComponent->IsFalling();
}
