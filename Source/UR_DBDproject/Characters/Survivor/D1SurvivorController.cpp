// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Survivor/D1SurvivorController.h"
#include "Data/D1InputData.h"
#include "System/D1AssetManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "D1GameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"
#include "GameFramework/CharacterMovementComponent.h"

AD1SurvivorController::AD1SurvivorController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AD1SurvivorController::BeginPlay()
{
	Super::BeginPlay();

	if (const UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("InputData"))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputData->InputMappingContext, 0);
		}
	}

	// 카메라 Pitch(위/아래) 제한 설정
	PlayerCameraManager->ViewPitchMin = -35.0f; // 최소 Pitch (아래 제한)
	PlayerCameraManager->ViewPitchMax = 35.0f;  // 최대 Pitch (위 제한)

	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());
	D1Survivor->GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AD1SurvivorController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("InputData"))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		auto MoveAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);

		auto LookAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);

		auto RunAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ThisClass::Input_RunStart);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ThisClass::Input_RunStop);

		auto CrouchAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Crouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::Input_StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::Input_StopCrouch);
	}
}

void AD1SurvivorController::Input_Move(const FInputActionValue& InputValue)
{
	FVector2D MovementVector = InputValue.Get<FVector2D>();

	if (!D1Survivor) return;

	if (MovementVector.X != 0)
	{
		FRotator Rotator = GetControlRotation();
		FVector Direction = UKismetMathLibrary::GetForwardVector(FRotator(0, Rotator.Yaw, 0));
		GetPawn()->AddMovementInput(Direction, MovementVector.X);
	}

	if (MovementVector.Y != 0)
	{
		FRotator Rotator = GetControlRotation();
		FVector Direction = UKismetMathLibrary::GetRightVector(FRotator(0, Rotator.Yaw, 0));
		GetPawn()->AddMovementInput(Direction, MovementVector.Y);
	}
}

void AD1SurvivorController::Input_Look(const FInputActionValue& InputValue)
{
	FVector2D LookVector = InputValue.Get<FVector2D>();
	// 좌우 회전
	AddYawInput(LookVector.X);
	// 상하 회전
	AddPitchInput(LookVector.Y);
}

void AD1SurvivorController::Input_RunStart()
{
	if (!D1Survivor)
	{
		return;
	}

	SetCreatureState(ECreatureState::Run);
	D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetRunSpeed();
}

void AD1SurvivorController::Input_RunStop()
{
	if (!D1Survivor)
	{
		return;
	}

	SetCreatureState(ECreatureState::None);
	D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetWalkSpeed();
}

void AD1SurvivorController::Input_StartCrouch()
{
	if (!D1Survivor)
	{
		return;
	}

	SetCreatureState(ECreatureState::Crouch);
	D1Survivor->Crouch();
}

void AD1SurvivorController::Input_StopCrouch()
{
	if (!D1Survivor)
	{
		return;
	}

	SetCreatureState(ECreatureState::None);
	D1Survivor->UnCrouch();
}

ECreatureState AD1SurvivorController::GetCreatureState()
{
	if (D1Survivor)
	{
		return D1Survivor->CreatureState;
	}

	return ECreatureState::None;
}

void AD1SurvivorController::SetCreatureState(ECreatureState InState)
{
	if (D1Survivor)
	{
		D1Survivor->CreatureState = InState;
	}
}
