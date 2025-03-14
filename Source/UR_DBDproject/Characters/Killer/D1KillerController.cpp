// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerController.h"
#include "Data/D1InputData.h"
#include "System/D1AssetManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "D1GameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Animation/D1KillerBaseAnim.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interactables/D1Generator.h"
#include "Interactables/D1VaultObject.h"
#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Transform.h"
#include "AbilitySystemBlueprintLibrary.h"

AD1KillerController::AD1KillerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AD1KillerController::BeginPlay()
{
	Super::BeginPlay();

	if (const UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("KillerInputData"))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputData->InputMappingContext, 0);
		}
	}

	D1Killer = Cast<AD1KillerBase>(GetCharacter());
	if (D1Killer)
	{
		TPVAnimInstance = Cast<UD1KillerBaseAnim>(D1Killer->GetCharacterMesh().Get()->GetAnimInstance());
		FPVAnimInstance = Cast<UD1KillerBaseAnim>(D1Killer->GetFPVMesh().Get()->GetAnimInstance());
		WolfAnimInstance = Cast<UD1KillerBaseAnim>(D1Killer->GetWolfMesh().Get()->GetAnimInstance());
		BatAnimInstance = Cast<UD1KillerBaseAnim>(D1Killer->GetBatMesh().Get()->GetAnimInstance());
	}
}

void AD1KillerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("KillerInputData"))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		auto MoveAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);

		auto LookAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);

		auto Attack1Action = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Attack1);
		EnhancedInputComponent->BindAction(Attack1Action, ETriggerEvent::Started, this, &ThisClass::Input_LeftClick);

		auto RightClickAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_RightClick);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &ThisClass::Input_RightClick);

		auto Skill1Action = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Skill1);
		EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Started, this, &ThisClass::Input_Skill1);
		EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Completed, this, &ThisClass::Input_OnCtrlReleased);

		auto BreakAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_SpaceBar);
		EnhancedInputComponent->BindAction(BreakAction, ETriggerEvent::Started, this, &ThisClass::HandleInteraction);

	}
}

void AD1KillerController::HandleGameplayEvent(FGameplayTag EventTag)
{
	if (EventTag == (D1GameplayTags::Killer_Attack_DetactStart))
	{
		if (D1Killer && D1Killer->AttackCollision)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_Attack_DetactStart"));
			D1Killer->AttackCollision->SetActive(true);
			D1Killer->AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}

	if (EventTag == (D1GameplayTags::Killer_Attack_DetactEnd))
	{
		if (D1Killer && D1Killer->AttackCollision)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_Attack_DetactEnd"));
			D1Killer->AttackCollision->SetActive(false);
			D1Killer->AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			D1Killer->SetbAttackSuccess(false);
		}
	}

	if (EventTag == (D1GameplayTags::Killer_Wolf_Attack_DetactStart))
	{
		if (D1Killer && D1Killer->WolfAttackCollision)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_Wolf_Attack_DetactStart"));
			D1Killer->WolfAttackCollision->SetActive(true);
			D1Killer->WolfAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}

	if (EventTag == (D1GameplayTags::Killer_Wolf_Attack_DetactEnd))
	{
		if (D1Killer && D1Killer->WolfAttackCollision)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_Wolf_Attack_DetactStart"));
			D1Killer->WolfAttackCollision->SetActive(false);
			D1Killer->WolfAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			D1Killer->SetbAttackSuccess(false);
		}
	}
}

void AD1KillerController::Input_Move(const FInputActionValue& InputValue)
{
	if (!D1Killer) return;

	SetCreatureState(ECreatureState::Walk);
	FVector2D MovementVector = InputValue.Get<FVector2D>();

	KillerSet = Cast<UD1KillerSet>(D1Killer->GetAttributeSet());

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

void AD1KillerController::Input_Look(const FInputActionValue& InputValue)
{
	if (!D1Killer || bIgnoreInputLook) return;

	FVector2D LookAxisVector = InputValue.Get<FVector2D>();

	// add yaw and pitch input to controller
	GetPawn()->AddControllerYawInput(LookAxisVector.X);
	GetPawn()->AddControllerPitchInput(-LookAxisVector.Y);
}

void AD1KillerController::Input_LeftClick(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

	if (bIsCtrlPressed)
	{
		UE_LOG(LogTemp, Log, TEXT("🎯 Ctrl + Left Click Activated!"));
		LeftClick_Transform();
		return;
	}

	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Attack);
		return;
	}

	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Warning, TEXT("WolfAttack"));
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Wolf_Attack);
	}
}

void AD1KillerController::Input_RightClick(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

	if (bIsCtrlPressed)
	{
		UE_LOG(LogTemp, Log, TEXT("🎯 Ctrl + Right Click Activated!"));
		RightClick_Transform();
	}
}

void AD1KillerController::Input_Skill1(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Log, TEXT("✅ Ctrl Key Pressed!"));
	bIsCtrlPressed = true;
}

void AD1KillerController::Input_OnCtrlReleased(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Log, TEXT("⛔ Ctrl Key Released!"));
	bIsCtrlPressed = false;
}

ECreatureState AD1KillerController::GetCreatureState()
{
	if (D1Killer)
	{
		return D1Killer->CreatureState;
	}

	return ECreatureState::None;
}

void AD1KillerController::SetCreatureState(ECreatureState InState)
{
	if (D1Killer)
	{
		D1Killer->CreatureState = InState;
	}
}

void AD1KillerController::LeftClick_Transform()
{
	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
	{
		D1Killer->SetPrevTransformState(EDraculaTransformationState::Dracula);
		D1Killer->SetCurrentTransformState(EDraculaTransformationState::Wolf);
	}
	else if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
	{
		D1Killer->SetPrevTransformState(EDraculaTransformationState::Wolf);
		D1Killer->SetCurrentTransformState(EDraculaTransformationState::Bat);
	}
	else if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Bat)
	{
		D1Killer->SetPrevTransformState(EDraculaTransformationState::Bat);
		D1Killer->SetCurrentTransformState(EDraculaTransformationState::Dracula);
	}

	D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Transform);
}

void AD1KillerController::RightClick_Transform()
{
	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
	{
		D1Killer->SetPrevTransformState(EDraculaTransformationState::Dracula);
		D1Killer->SetCurrentTransformState(EDraculaTransformationState::Bat);
	}
	else if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
	{
		D1Killer->SetPrevTransformState(EDraculaTransformationState::Wolf);
		D1Killer->SetCurrentTransformState(EDraculaTransformationState::Dracula);
	}
	else if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Bat)
	{
		D1Killer->SetPrevTransformState(EDraculaTransformationState::Bat);
		D1Killer->SetCurrentTransformState(EDraculaTransformationState::Wolf);
	}
	D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Transform);
}

void AD1KillerController::HandleInteraction()
{
	if (!D1Killer) return;

	AActor* DetectedObject = D1Killer->GetDetectedObject();
	if (!DetectedObject) return;

	UE_LOG(LogTemp, Log, TEXT("✅ DetectedObject: %s, Class: %s"),
		*DetectedObject->GetName(), *DetectedObject->GetClass()->GetName());
	if (D1Killer->GetCurrentHook() && CarriedSurvivor != nullptr)
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_HookSurvivor);
	}
	else if (D1Killer->GetDetectedCrawlSurvivor() && CarriedSurvivor == nullptr)
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_PickUpSurvivor);
	}
	else if (D1Killer->GetCurrentPallet())
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_DestroyPallet);
	}
	else if (D1Killer->GetCurrentGenerator())
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_DamageGenerator);
	}
	else if (D1Killer->GetVaultTarget())
	{
		if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
		{
			D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_VaultWindow);
		}
		if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
		{
			D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Wolf_VaultWindow);
		}
	}
}

