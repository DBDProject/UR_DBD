// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/D1PlayerController.h"
#include "Data/D1InputData.h"
#include "System/D1AssetManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "D1GameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/D1CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AD1PlayerController::AD1PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
void AD1PlayerController::BeginPlay()
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
}

void AD1PlayerController::SetupInputComponent()
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
	}
}

void AD1PlayerController::Input_Move(const FInputActionValue& InputValue)
{
	FVector2D MovementVector = InputValue.Get<FVector2D>();

	AD1CharacterBase* PlayerCharacter = Cast<AD1CharacterBase>(GetPawn());

	if (!PlayerCharacter) return;

	// Shift 키 상태에 따라 속도 변경
	float CurrentSpeed = PlayerCharacter->IsRunning() ? PlayerCharacter->GetRunSpeed() : PlayerCharacter->GetWalkSpeed();
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
	
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

void AD1PlayerController::Input_Look(const FInputActionValue& InputValue)
{
	FVector2D LookVector = InputValue.Get<FVector2D>();
	// 좌우 회전
	AddYawInput(LookVector.X);
	// 상하 회전
	AddPitchInput(LookVector.Y);
}
void AD1PlayerController::Input_RunStart()
{
	if (AD1CharacterBase* PlayerCharacter = Cast<AD1CharacterBase>(GetPawn()))
	{
		PlayerCharacter->SetRunning(true);
	}
}

void AD1PlayerController::Input_RunStop()
{
	if (AD1CharacterBase* PlayerCharacter = Cast<AD1CharacterBase>(GetPawn()))
	{
		PlayerCharacter->SetRunning(false);
	}
}
