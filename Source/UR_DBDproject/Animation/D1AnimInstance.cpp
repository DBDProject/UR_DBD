// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1AnimInstance.h"

UD1AnimInstance::UD1AnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UD1AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//Character = Cast<AR1Character>(TryGetPawnOwner());

	//if (Character)
	//	MovementComponent = Character->GetCharacterMovement();
}

void UD1AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//if (Character == nullptr)
	//	return;

	//if (MovementComponent == nullptr)
	//	return;

	//Velocity = MovementComponent->Velocity;
	//GroundSpeed = Velocity.Size2D();

	//bShouldMove = (GroundSpeed > 3.f && MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector);
	//bIsFalling = MovementComponent->IsFalling();
}