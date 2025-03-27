// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerController.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Killer/D1KillerState.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Data/D1InputData.h"
#include "System/D1AssetManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "D1GameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/D1KillerBaseAnim.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interactables/D1Generator.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"

AD1KillerController::AD1KillerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void AD1KillerController::BeginPlay()
{
	Super::BeginPlay();
	SetReplicates(true);

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
		WolfAnimInstance = Cast<UD1KillerBaseAnim>(D1Killer->GetMesh()->GetAnimInstance());
		BatAnimInstance = Cast<UD1KillerBaseAnim>(D1Killer->GetBatMesh().Get()->GetAnimInstance());
		SetupInputComponent();
	}
}

void AD1KillerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!D1Killer)
	{
		return;
	}

	UD1AbilitySystemComponent* AbilitySystemComponent = D1Killer->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ AbilitySystemComponent is NULL!"));
		return;
	}

	AbilitySystemComponent->AddCharacterAbilities(D1Killer->StartupAbilities);

	if (UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("KillerInputData"))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		auto MoveAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);

		auto LookAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);

		auto Attack1Action = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Attack1);
		EnhancedInputComponent->BindAction(Attack1Action, ETriggerEvent::Started, this, &ThisClass::Input_LeftClick);

		RightClickAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_RightClick);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &ThisClass::Input_RightClick);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Completed, this, &ThisClass::Input_RightClickRelease);
		AbilitySystemComponent->SetInputBinding(RightClickAction, D1GameplayTags::Killer_Ability_Dracula_PowerAttack);

		WolfPounce_InputAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_KeyboardF);
		EnhancedInputComponent->BindAction(WolfPounce_InputAction, ETriggerEvent::Started, this, &ThisClass::Input_KeyboardF);
		EnhancedInputComponent->BindAction(WolfPounce_InputAction, ETriggerEvent::Completed, this, &ThisClass::Input_FRelease);
		AbilitySystemComponent->SetInputBinding(WolfPounce_InputAction, D1GameplayTags::Killer_Ability_Wolf_PowerAttack);

		auto Skill1Action = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Skill1); //Ctrl
		EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Started, this, &ThisClass::Input_Skill1);
		EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Completed, this, &ThisClass::Input_OnCtrlReleased);

		auto BreakAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_SpaceBar); //Spacebar
		EnhancedInputComponent->BindAction(BreakAction, ETriggerEvent::Started, this, &ThisClass::HandleInteraction);

		auto DropAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Drop); // R
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &ThisClass::Input_Drop);

	}

}

void AD1KillerController::HandleGameplayEvent(FGameplayTag EventTag)
{
	if (EventTag == (D1GameplayTags::Killer_Attack_DetactStart))
	{
		if (D1Killer)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_Attack_DetactStart"));
			D1Killer->PerformDraculaAttackTrace();
		}
		return;
	}

	if (EventTag == (D1GameplayTags::Killer_Wolf_Attack_DetactStart))
	{
		if (D1Killer)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_Wolf_Attack_DetactStart"));
			D1Killer->PerformWolfAttackTrace();
		}
		return;
	}

	if (EventTag == (D1GameplayTags::Killer_PowerAttack_DetactStart))
	{
		if (D1Killer && D1Killer->PowerAttackCollision)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_PowerAttack_DetactStart"));
			D1Killer->PowerAttackCollision->SetActive(true);
			D1Killer->PowerAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		return;
	}

	if (EventTag == (D1GameplayTags::Killer_PowerAttack_DetactEnd))
	{
		if (D1Killer && D1Killer->PowerAttackCollision)
		{
			UE_LOG(LogTemp, Warning, TEXT("Killer_PowerAttack_DetactEnd"));
			D1Killer->PowerAttackCollision->SetActive(false);
			D1Killer->PowerAttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			D1Killer->SetbAttackSuccess(false);
		}
		return;
	}
}

void AD1KillerController::Input_Move(const FInputActionValue& InputValue)
{
	if (!D1Killer) return;

	SetCreatureState(ECreatureState::Walk);
	FVector2D MovementVector = InputValue.Get<FVector2D>();

	D1Killer->KillerSet = Cast<UD1KillerSet>(D1Killer->GetAttributeSet());

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
		if (D1Killer->GetCarriedSurvivor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack"));
			D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_CarryAttack);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack"));
			D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_Attack);
			return;
		}
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
		return;
	}

	if (!D1Killer->GetCarriedSurvivor())
	{
		if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Warning, TEXT("PowerAttack"));
			if (D1Killer->GetAbilitySystemComponent())
			{
				D1Killer->GetAbilitySystemComponent()->OnAbilityInputPressed(RightClickAction);
			}
			return;
		}
	}
}

void AD1KillerController::Input_RightClickRelease(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerAttack Release"));
		if (D1Killer->GetAbilitySystemComponent())
		{
			D1Killer->GetAbilitySystemComponent()->OnAbilityInputReleased(RightClickAction);
		}
		return;
	}
}

void AD1KillerController::Input_KeyboardF(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf PowerAttack"));
		if (D1Killer->GetAbilitySystemComponent())
		{
			comboIndex++;
			D1Killer->GetAbilitySystemComponent()->OnAbilityInputPressed(WolfPounce_InputAction);
			bComboAttackable = false;
		}
		return;
	}
}

void AD1KillerController::ResetCombo()
{
	bComboAttackable = false;
	comboIndex = 0;
	GetWorld()->GetTimerManager().ClearTimer(ComboDashTimer);
}

void AD1KillerController::Input_FRelease(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf PowerAttack Release"));
		if (D1Killer->GetAbilitySystemComponent())
		{
			D1Killer->GetAbilitySystemComponent()->OnAbilityInputReleased(WolfPounce_InputAction);
		}
		return;
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

void AD1KillerController::Input_Drop(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Log, TEXT("⛔ Drop Key Released!"));

	if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula && D1Killer->GetCarriedSurvivor() != nullptr)
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_DropSurvivor);
	}
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
	if (!bTransform)
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
		bTransform = true;
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_Transform);
	}
}

void AD1KillerController::RightClick_Transform()
{
	if (!bTransform)
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
		bTransform = true;
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_Transform);
	}
}

void AD1KillerController::HandleInteraction()
{
	if (!D1Killer) return;

	AActor* DetectedObject = D1Killer->GetDetectedObject();
	if (!DetectedObject) return;

	UE_LOG(LogTemp, Log, TEXT("✅ DetectedObject: %s, Class: %s"),
		*DetectedObject->GetName(), *DetectedObject->GetClass()->GetName());


	auto EnsureDraculaFormAndActivate = [&](FGameplayTag AbilityTag)
		{
			if (D1Killer->GetCurrentTransformState() != EDraculaTransformationState::Dracula)
			{
				// 변신 후 어빌리티 실행
				D1Killer->SetPrevTransformState(D1Killer->GetCurrentTransformState());
				D1Killer->SetCurrentTransformState(EDraculaTransformationState::Dracula);
				D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_Transform);

				// 변신이 완료될 때까지 딜레이를 준 후 어빌리티 실행
				FTimerHandle TransformTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TransformTimerHandle, [this, AbilityTag]()
					{
						if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
						{
							D1Killer->ActivateAbility(AbilityTag);
						}
					}, 1.25f, false);
			}
			else
			{
				D1Killer->ActivateAbility(AbilityTag);
			}
		};

	if (D1Killer->GetCurrentHook() && D1Killer->GetCarriedSurvivor() != nullptr)
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_HookSurvivor);
		return;
	}

	if (D1Killer->GetDetectedCrawlSurvivor() && D1Killer->GetCarriedSurvivor() == nullptr)
	{
		AD1SurvivorBase* Survivor = D1Killer->GetDetectedCrawlSurvivor();
		if (Survivor->GetSurvivorState() == ESurvivorState::Crawl)
		{
			EnsureDraculaFormAndActivate(D1GameplayTags::Killer_Ability_Dracula_PickUpSurvivor);
			return;
		}
	}

	if (D1Killer->GetCurrentPallet() && !D1Killer->GetCarriedSurvivor())
	{
		EnsureDraculaFormAndActivate(D1GameplayTags::Killer_Ability_Dracula_DestroyPallet);
		return;
	}

	if (D1Killer->GetCurrentGenerator() && !D1Killer->GetCarriedSurvivor())
	{
		AD1Generator* generator = D1Killer->GetCurrentGenerator();
		if (generator->GetRepairProgress() >= 100.0f)
		{
			EnsureDraculaFormAndActivate(D1GameplayTags::Killer_Ability_Dracula_DamageGenerator);
			return;

		}
	}

	if (D1Killer->GetVaultTarget() && !D1Killer->GetCarriedSurvivor())
	{
		if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
		{
			D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_VaultWindow);
		}
		if (D1Killer->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
		{
			D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Wolf_VaultWindow);
		}
		return;
	}
}
