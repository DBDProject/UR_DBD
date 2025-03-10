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
#include "Interactables//D1Pallet.h"
#include "Interactables//D1Hook.h"
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

		CurrentTransformState = EDraculaTransformationState::Dracula;
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
		}
	}

	if (EventTag.MatchesTag(D1GameplayTags::Killer_PalletEnd))
	{
		EndDestroyPallet();
	}

	if (EventTag.MatchesTag(D1GameplayTags::Killer_PickUpEnd))
	{
		EndPickUpPlayer();
	}

	if (EventTag.MatchesTag(D1GameplayTags::Killer_HookEnd))
	{
		EndHookPlayer();
	}
	if (EventTag.MatchesTag(D1GameplayTags::Killer_VaultWindowEnd))
	{
		EndVault();
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
	if (!D1Killer) return;

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

	if (CurrentTransformState == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Attack);
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
	if (CurrentTransformState == EDraculaTransformationState::Dracula)
	{
		PrevTransformState = EDraculaTransformationState::Dracula;
		CurrentTransformState = EDraculaTransformationState::Wolf;
	}
	else if (CurrentTransformState == EDraculaTransformationState::Wolf)
	{
		PrevTransformState = EDraculaTransformationState::Wolf;
		CurrentTransformState = EDraculaTransformationState::Bat;
	}
	else if (CurrentTransformState == EDraculaTransformationState::Bat)
	{
		PrevTransformState = EDraculaTransformationState::Bat;
		CurrentTransformState = EDraculaTransformationState::Dracula;
	}

	D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Transform);
}

void AD1KillerController::RightClick_Transform()
{
	if (CurrentTransformState == EDraculaTransformationState::Dracula)
	{
		PrevTransformState = EDraculaTransformationState::Dracula;
		CurrentTransformState = EDraculaTransformationState::Bat;
	}
	else if (CurrentTransformState == EDraculaTransformationState::Wolf)
	{
		PrevTransformState = EDraculaTransformationState::Wolf;
		CurrentTransformState = EDraculaTransformationState::Dracula;
	}
	else if (CurrentTransformState == EDraculaTransformationState::Bat)
	{
		PrevTransformState = EDraculaTransformationState::Bat;
		CurrentTransformState = EDraculaTransformationState::Wolf;
	}
	D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_Transform);
}

//void AD1KillerController::StartDamageGenerator()
//{
//	AD1Generator* Generator = D1Killer->GetCurrentGenerator();
//	if (!Generator)
//		return;
//
//	if (Generator->GetRepairProgress() == 0.0f || Generator->GetRepairProgress() >= 100.0f
//		|| Generator->GetCurrentState() == EGeneratorState::Breaking)
//		return;
//
//	EGeneratorInteractionPosition Pos =
//		Generator->FindInteractionPosition(D1Killer);
//
//	MoveToGeneratorPosition(Pos);
//
//	Generator->SetCurrentState(EGeneratorState::Breaking);
//
//	FPVAnimInstance->Montage_Play(FPV_DamageGenerator, 1.0f);
//	TPVAnimInstance->Montage_Play(TPV_DamageGenerator, 1.0f);
//
//	UE_LOG(LogTemp, Warning, TEXT("Damage Generator"));
//}
//
//void AD1KillerController::EndDamageGenerator()
//{
//	AD1Generator* Generator = D1Killer->GetCurrentGenerator();
//	if (!D1Killer || !Generator)
//		return;
//
//	Generator->OnDamage();
//
//	Generator->SetCurrentState(EGeneratorState::Idle);
//}
//
//void AD1KillerController::MoveToGeneratorPosition(EGeneratorInteractionPosition Position)
//{
//	if (!D1Killer || !D1Killer->GetCurrentGenerator()) return;
//
//	AD1Generator* Generator = D1Killer->GetCurrentGenerator();
//	FVector GeneratorLocation = Generator->GetActorLocation();
//	FVector ForwardVector = Generator->GetActorForwardVector();
//	FVector RightVector = Generator->GetActorRightVector();
//	FVector TargetLocation;
//
//	// 플레이어를 발전기 위치로 이동
//	switch (Position)
//	{
//	case EGeneratorInteractionPosition::Front:
//		TargetLocation = GeneratorLocation + ForwardVector * 94.f;
//		break;
//	case EGeneratorInteractionPosition::Back:
//		TargetLocation = GeneratorLocation - ForwardVector * 110.f;
//		break;
//	case EGeneratorInteractionPosition::Left:
//		TargetLocation = GeneratorLocation - RightVector * 80.f;
//		break;
//	case EGeneratorInteractionPosition::Right:
//		TargetLocation = GeneratorLocation + RightVector * 85.f;
//		break;
//	default:
//		return;
//	}
//	TargetLocation.Z += 88.f;  // Z 값 증가
//
//	D1Killer->SetActorLocation(TargetLocation);
//
//	// 플레이어 방향을 발전기로 조정 (자동 회전)
//	FRotator LookAtRotation = (GeneratorLocation - TargetLocation).Rotation();
//	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
//	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
//	D1Killer->SetActorRotation(LookAtRotation);
//}

void AD1KillerController::StartDestroyPallet()
{
	AD1Pallet* Pallet = D1Killer->GetCurrentPallet();
	if (!Pallet)
		return;

	if (Pallet->GetCurrentState() == EPalletState::Up)
		return;

	EPalletLocation PalletLocation = Pallet->MovePlayerToInteractionPoint(D1Killer);

	if (PalletLocation == EPalletLocation::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalletLocation None"));
		return;
	}

	Pallet->SetCurrentState(EPalletState::Destroyed);
	TPVAnimInstance->Montage_Play(TPV_DestroyPallet, 1.0f);
	FPVAnimInstance->Montage_Play(FPV_DestroyPallet, 1.0f);

	UE_LOG(LogTemp, Warning, TEXT("Destroy Pallet"));
}

void AD1KillerController::EndDestroyPallet()
{
	AD1Pallet* Pallet = D1Killer->GetCurrentPallet();
	if (!D1Killer || !Pallet)
		return;

	if (Pallet->GetCurrentState() != EPalletState::Destroyed)
		return;

	Pallet->OnDestroy();
}

void AD1KillerController::StartPickUpPlayer()
{
	AD1SurvivorBase* Survivor = D1Killer->GetDetectedCrawlSurvivor();
	if (!Survivor)
		return;
	
	Survivor->TakePickUpFromKiller(D1Killer);

	//FVector TargetLocation = Survivor->GetMesh()->GetSocketLocation(FName("jaw"));
	//UE_LOG(LogTemp, Warning, TEXT("TargetLocation: X = %.2f, Y = %.2f, Z = %.2f"),
	//	TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
	//if (TargetLocation.IsZero())
	//{
	//	TargetLocation = Survivor->GetMesh()->GetSocketLocation(FName("nose"));
	//}
	//FRotator LookAtRotation = (TargetLocation - D1Killer->GetActorLocation()).Rotation();
	//LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	//LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지

	//D1Killer->SetActorRotation(LookAtRotation);
	//SetControlRotation(LookAtRotation);
	
	TPVAnimInstance->Montage_Play(TPV_PickUpSurvivor, 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PickUpSurvivor, 1.0f);

	CarriedSurvivor = Survivor;

	UE_LOG(LogTemp, Warning, TEXT("생존자 픽업"));
}

void AD1KillerController::EndPickUpPlayer()
{
	if (!D1Killer)
		return;

	TPVAnimInstance->SetIsCarryingSurvivor(true);
	FPVAnimInstance->SetIsCarryingSurvivor(true);

	UE_LOG(LogTemp, Warning, TEXT("생존자 픽업 끝"));
}

void AD1KillerController::StartHookPlayer()
{
	AD1Hook* Hook = D1Killer->GetCurrentHook();
	if (!Hook)
		return;

	if (!CarriedSurvivor)
		return;

	CarriedSurvivor->OnHooked(Hook);

	TPVAnimInstance->Montage_Play(TPV_HookSurvivor, 1.0f);
	FPVAnimInstance->Montage_Play(FPV_HookSurvivor, 1.0f);

	UE_LOG(LogTemp, Warning, TEXT("생존자 훅 시작"));
}

void AD1KillerController::EndHookPlayer()
{
	if (!D1Killer)
		return;

	CarriedSurvivor = nullptr;

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	UE_LOG(LogTemp, Warning, TEXT("생존자 훅 끝"));
}

void AD1KillerController::StartVault()
{
	AD1VaultObject* VaultObj = D1Killer->GetVaultTarget();
	if (!VaultObj)
		return;

	VaultObj->MoveToVaultInteractionLocation(D1Killer);

	D1Killer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
	FPVAnimInstance->Montage_Play(FPV_VaultWindow, 1.0f);
	TPVAnimInstance->Montage_Play(TPV_VaultWindow, 1.0f);

}

void AD1KillerController::EndVault()
{
	AD1VaultObject* VaultObj = D1Killer->GetVaultTarget();
	if (!VaultObj)
		return;
	FVector StartLocation = VaultObj->GetStartPos();
	FVector TargetLocation = VaultObj->GetTargetPos();

	D1Killer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	
	D1Killer->SetActorLocation(TargetLocation);


}

void AD1KillerController::VaultUpdate()
{

}

void AD1KillerController::HandleInteraction()
{
	if (!D1Killer) return;

	AActor* DetectedObject = D1Killer->GetDetectedObject();
	if (!DetectedObject) return;

	if (D1Killer->GetCurrentHook() && CarriedSurvivor != nullptr)
	{
		StartHookPlayer();
	}
	else if (D1Killer->GetDetectedCrawlSurvivor())
	{
		StartPickUpPlayer();
	}
	else if (D1Killer->GetCurrentPallet())
	{
		StartDestroyPallet();
	}
	else if (D1Killer->GetCurrentGenerator())
	{
		D1Killer->ActivateAbility(D1GameplayTags::Killer_Ability_DamageGenerator);
	}
	else if (D1Killer->GetVaultTarget())
	{
		UE_LOG(LogTemp, Warning, TEXT("Vault"));
		StartVault();
	}
}

void AD1KillerController::SetIgnoreInput(bool bEnable)
{
	if (D1Killer)
	{
		if (bEnable)
		{
			SetIgnoreMoveInput(true);  // 이동 차단
			SetIgnoreLookInput(true);  // 시점 이동 차단
			UE_LOG(LogTemp, Log, TEXT("🔴 변신 모드 ON (이동 및 공격 입력 차단)"));
		}
		else
		{
			SetIgnoreMoveInput(false);
			SetIgnoreLookInput(false);
			UE_LOG(LogTemp, Log, TEXT("🟢 변신 모드 OFF (입력 복구)"));
		}
	}
}