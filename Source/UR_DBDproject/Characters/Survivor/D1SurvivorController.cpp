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
#include "Interactables/D1Generator.h"
#include "Animation/D1SurvivorBaseAnim.h"

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
	if (D1Survivor)
	{
		D1Survivor->GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
		CachedAnimInstance = Cast<UD1SurvivorBaseAnim>(D1Survivor->GetMesh()->GetAnimInstance());
	}
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

		auto InteractAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Interact);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Input_StartInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ThisClass::Input_StopInteract);
	}
}

void AD1SurvivorController::Input_Move(const FInputActionValue& InputValue)
{
	FVector2D MovementVector = InputValue.Get<FVector2D>();

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

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
	if (!D1Survivor || !D1Survivor->GetSurvivoreSet()) return;

	SetCreatureState(ECreatureState::Run);
	D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetRunSpeed();
}

void AD1SurvivorController::Input_RunStop()
{
	if (!D1Survivor || !D1Survivor->GetSurvivoreSet()) return;

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

void AD1SurvivorController::Input_StartInteract()
{
	if (!D1Survivor) return;

	if (AD1Generator* Generator = Cast<AD1Generator>(D1Survivor->GetDetectedObject()))
	{
		StartRepair();
	}
}

void AD1SurvivorController::Input_StopInteract()
{
	if (!D1Survivor) return;

	if (AD1Generator* Generator = Cast<AD1Generator>(D1Survivor->GetDetectedObject()))
	{
		StopRepair();
	}
}

void AD1SurvivorController::StartRepair()
{
	if (!D1Survivor || !D1Survivor->GetCurrentGenerator()) return;


	if (CachedAnimInstance.IsValid())
	{
		// 플레이어 위치 판별
		EGeneratorInteractionPosition Position = 
			D1Survivor->GetCurrentGenerator()->FindInteractionPosition(D1Survivor);

		// 플레이어 위치 이동
		MoveToGeneratorPosition(Position);

		CachedAnimInstance.Get()->SetInteractionPosition(Position);
		CachedAnimInstance.Get()->SetIsRepairing(true);

		// 이동 입력 차단
		D1Survivor->GetCharacterMovement()->DisableMovement();
		D1Survivor->GetCharacterMovement()->StopMovementImmediately();

		D1Survivor->GetCurrentGenerator()->StartRepair(D1Survivor, Position);
	}
}

void AD1SurvivorController::StopRepair()
{
	if (!D1Survivor || !D1Survivor->GetCurrentGenerator()) return;

	if (CachedAnimInstance.IsValid())
	{
		CachedAnimInstance.Get()->SetIsRepairing(false);

		// 이동 가능하게 변경
		D1Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		D1Survivor->GetCurrentGenerator()->StopRepair();
	}
}

void AD1SurvivorController::MoveToGeneratorPosition(EGeneratorInteractionPosition Position)
{
	if (!D1Survivor || !D1Survivor->GetCurrentGenerator()) return;

	AD1Generator* Generator = D1Survivor->GetCurrentGenerator();
	FVector GeneratorLocation = Generator->GetActorLocation();
	FVector ForwardVector = Generator->GetActorForwardVector();
	FVector RightVector = Generator->GetActorRightVector();
	FVector TargetLocation;

	// 플레이어를 발전기 위치로 이동
	switch (Position)
	{
	case EGeneratorInteractionPosition::Front:
		TargetLocation = GeneratorLocation + ForwardVector * 100.f;
		break;
	case EGeneratorInteractionPosition::Back:
		TargetLocation = GeneratorLocation - ForwardVector * 90.f;
		break;
	case EGeneratorInteractionPosition::Left:
		TargetLocation = GeneratorLocation - RightVector * 80.f;
		break;
	case EGeneratorInteractionPosition::Right:
		TargetLocation = GeneratorLocation + RightVector * 80.f;
		break;
	default:
		return;
	}
	TargetLocation.X -= 6.f;  // X 값 보정
	TargetLocation.Z += 88.f;  // Z 값 증가

	D1Survivor->SetActorLocation(TargetLocation);

	// 플레이어 방향을 발전기로 조정 (자동 회전)
	FRotator LookAtRotation = (GeneratorLocation - TargetLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
	D1Survivor->SetActorRotation(LookAtRotation);
}

ECreatureState AD1SurvivorController::GetCreatureState()
{
	return D1Survivor ? D1Survivor->CreatureState : ECreatureState::None;
}

void AD1SurvivorController::SetCreatureState(ECreatureState InState)
{
	if (D1Survivor)
	{
		D1Survivor->CreatureState = InState;
	}
}
