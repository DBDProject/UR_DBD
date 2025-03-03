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
#include "Interactables/D1VaultObject.h"
#include "Components/CapsuleComponent.h"
#include "Interactables/D1Pallet.h"

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

		// 좌클릭
		auto InteractAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Interact);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::Input_StartInteract_LeftClick);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ThisClass::Input_StopInteract_LeftClick);

		// 스페이스
		auto InteractAction2 = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Parkour);
		EnhancedInputComponent->BindAction(InteractAction2, ETriggerEvent::Started, this, &ThisClass::Input_StartInteract_Space);

		// Test(1)
		auto TestAction2 = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_TestInput);
		EnhancedInputComponent->BindAction(TestAction2, ETriggerEvent::Started, this, &ThisClass::Input_StartTestInput_1);
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

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl) return;

	SetCreatureState(ECreatureState::Run);

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetRunSpeed();
	}
	else if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetInjRunSpeed();
	}
}

void AD1SurvivorController::Input_RunStop()
{
	if (!D1Survivor || !D1Survivor->GetSurvivoreSet()) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl) return;

	SetCreatureState(ECreatureState::None);

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetWalkSpeed();
	}
	else if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor->GetCharacterMovement()->MaxWalkSpeed = D1Survivor->GetSurvivoreSet()->GetInjWalkSpeed();
	}
}

void AD1SurvivorController::Input_StartCrouch()
{
	if (!D1Survivor) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl) return;

	if (GetCreatureState() == ECreatureState::Parkour) return;

	SetCreatureState(ECreatureState::Crouch);
	D1Survivor->Crouch();
}

void AD1SurvivorController::Input_StopCrouch()
{
	if (!D1Survivor) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl) return;

	SetCreatureState(ECreatureState::None);
	D1Survivor->UnCrouch();
}

void AD1SurvivorController::Input_StartInteract_LeftClick()
{
	if (!D1Survivor) return;
	if (GetCreatureState() == ECreatureState::Parkour) return;

	if (AD1Generator* Generator = Cast<AD1Generator>(D1Survivor->GetDetectedObject()))
	{
		StartRepair();
	}

	if (AD1SurvivorBase* TargetSurvivor = Cast<AD1SurvivorBase>(D1Survivor->GetDetectedObject()))
	{
		StartHealing(TargetSurvivor);
	}
}

void AD1SurvivorController::Input_StopInteract_LeftClick()
{
	if (!D1Survivor) return;

	if (AD1Generator* Generator = Cast<AD1Generator>(D1Survivor->GetDetectedObject()))
	{
		StopRepair();
	}

	if (AD1SurvivorBase* TargetSurvivor = Cast<AD1SurvivorBase>(D1Survivor->GetDetectedObject()))
	{
		StopHealing(TargetSurvivor);
	}
}

void AD1SurvivorController::Input_StartInteract_Space()
{
	if (!D1Survivor) return;

	if (GetCreatureState() == ECreatureState::Parkour) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl) return;

	if (AD1VaultObject* VaultTarget = Cast<AD1VaultObject>(D1Survivor->GetVaultTarget()))
	{
		// 현재 속도 가져오기
		float CurrentSpeed = D1Survivor->GetVelocity().Size();

		// 속도 기준으로 파쿠르 속도 결정
		EVaultType VaultType = EVaultType::Medium; // 기본값 Medium
		if (CurrentSpeed > 300.0f) VaultType = EVaultType::Fast;  // 달리기 속도
		else if (CurrentSpeed < 100.0f) VaultType = EVaultType::Slow; // 웅크리기 속도

		PerformVault(VaultType);

		return;
	}

	if (AD1Pallet* Pallet = Cast<AD1Pallet>(D1Survivor->GetCurrentPallet()))
	{
		if (Pallet->GetCurrentState() == EPalletState::Up)
		{
			DropPallet();
		}
		else
		{
			if (!bCanVaultAfterDrop)
			{
				UE_LOG(LogTemp, Warning, TEXT("VaultPallet 사용 불가 (쿨다운)"));
				return;
			}

			VaultPallet();
		}
		return;
	}
}

void AD1SurvivorController::Input_StartTestInput_1()
{
	UE_LOG(LogTemp, Warning, TEXT("TakeDamageFromKiller"));
	D1Survivor->TakeDamageFromKiller();
}

void AD1SurvivorController::StartRepair()
{
	if (!D1Survivor || !D1Survivor->GetCurrentGenerator()) return;

	if (D1Survivor->GetCurrentGenerator()->GetIsRepairBlocked() == true ||
		D1Survivor->GetCurrentGenerator()->GetRepairProgress() >= 100.f) return;

	if (CachedAnimInstance.IsValid())
	{
		D1Survivor->SetIsFail(false);

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
	
	if (D1Survivor->GetCurrentGenerator()->GetIsFail() == true) return;

	if (CachedAnimInstance.IsValid())
	{
		CachedAnimInstance.Get()->SetIsRepairing(false);

		// 이동 가능하게 변경
		D1Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		D1Survivor->GetCurrentGenerator()->StopRepair(D1Survivor);
	}
}

void AD1SurvivorController::StartHealing(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor || !TargetSurvivor) return;

	if (TargetSurvivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		UE_LOG(LogTemp, Warning, TEXT("대상 생존자는 치료가 필요하지 않습니다!"));
		StopHealing(TargetSurvivor);
		return;
	}

	if (!TargetSurvivor->GetCanBeHealed())
	{
		UE_LOG(LogTemp, Warning, TEXT("대상 생존자는 현재 치료 불가 상태입니다!"));
		StopHealing(TargetSurvivor);
		return;
	}

	// 플레이어 방향 조정 (자동 회전)
	FRotator LookAtRotation = 
		(TargetSurvivor->GetActorLocation() - D1Survivor->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
	D1Survivor->SetActorRotation(LookAtRotation);

	if (CachedAnimInstance.IsValid())
	{
		CachedAnimInstance->SetIsHealing(true);
		CachedAnimInstance->SetHealingTargetState(TargetSurvivor->GetSurvivorState());

		// 입력 차단
		D1Survivor->GetCharacterMovement()->DisableMovement();

		TargetSurvivor->BeingHealing(D1Survivor);
	}
}

void AD1SurvivorController::StopHealing(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor || !TargetSurvivor) return;

	if (CachedAnimInstance.IsValid())
	{
		CachedAnimInstance->SetIsHealing(false);

		// 입력 차단 해제
		D1Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		CachedAnimInstance->SetHealingTargetState(ESurvivorState::None);

		TargetSurvivor->StopBeingHealing();
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
		TargetLocation = GeneratorLocation + ForwardVector * 94.f;
		break;
	case EGeneratorInteractionPosition::Back:
		TargetLocation = GeneratorLocation - ForwardVector * 110.f;
		break;
	case EGeneratorInteractionPosition::Left:
		TargetLocation = GeneratorLocation - RightVector * 80.f;
		break;
	case EGeneratorInteractionPosition::Right:
		TargetLocation = GeneratorLocation + RightVector * 85.f;
		break;
	default:
		return;
	}
	TargetLocation.Z += 88.f;  // Z 값 증가

	D1Survivor->SetActorLocation(TargetLocation);

	// 플레이어 방향을 발전기로 조정 (자동 회전)
	FRotator LookAtRotation = (GeneratorLocation - TargetLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
	D1Survivor->SetActorRotation(LookAtRotation);
}

void AD1SurvivorController::PerformVault(EVaultType VaultType)
{
	if (!D1Survivor || !D1Survivor->GetVaultTarget()) return;

	if (!VaultMontage) return;

	FName SectionName;
	switch (VaultType)
	{
	case EVaultType::Slow:
		SectionName = "Vault_Slow";
		break;
	case EVaultType::Medium:
		SectionName = "Vault_Mid";
		break;
	case EVaultType::Fast:
		SectionName = "Vault_Fast";
		break;
	default:
		SectionName = "Vault_Mid";
		break;
	}

	// 애니메이션 실행
 	if (CachedAnimInstance.IsValid())
	{
		SetCreatureState(ECreatureState::Parkour);

		D1Survivor->MoveToVaultStartPosition();
		D1Survivor->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
		D1Survivor->PlayAnimMontage(VaultMontage, 1.0f, SectionName);
	}
}

void AD1SurvivorController::DropPallet()
{
	AD1Pallet* Pallet = D1Survivor->GetCurrentPallet();
	if (!D1Survivor || !Pallet) return;

	if (Pallet->GetCurrentState() == EPalletState::Down) return;

	// 현재 속도 가져오기
	float CurrentSpeed = D1Survivor->GetVelocity().Size();

	// 플레이어 위치, 방향 이동
	EPalletLocation PalletLocation = Pallet->MovePlayerToInteractionPoint(D1Survivor);

	if (PalletLocation == EPalletLocation::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalletLocation None"));
		return;
	}

	FName SectionName;
	if (CurrentSpeed < 300.f) // 서있거나 걷고있을 때
	{
		
		SectionName = (PalletLocation == EPalletLocation::LT) ? "StandPullDownLT" : "StandPullDownRT";
		
	}
	else
	{
		// TEMP
		SectionName = (PalletLocation == EPalletLocation::LT) ? "StandPullDownLT" : "StandPullDownRT";

		//SectionName = (PalletLocation == EPalletLocation::LT) ? "WalkPullDownLT" : "WalkPullDownRT";
	}


	// 애니메이션 실행
	if (CachedAnimInstance.IsValid())
	{
		D1Survivor->PlayAnimMontage(PalletMontage, 1.0f, SectionName);

		Pallet->SetCurrentState(EPalletState::Down); 
		
		bCanVaultAfterDrop = false;
		GetWorld()->GetTimerManager().SetTimer(VaultCooldownTimer, this, &AD1SurvivorController::EnableVaultAfterDrop, 1.0f, false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Pallet Drop"));
}

void AD1SurvivorController::VaultPallet()
{
	AD1Pallet* Pallet = D1Survivor->GetCurrentPallet();
	if (!D1Survivor || !Pallet) return;

	if (Pallet->GetCurrentState() == EPalletState::Up) return;


	// 현재 속도 가져오기
	float CurrentSpeed = D1Survivor->GetVelocity().Size();

	// 플레이어 위치, 방향 이동
	EPalletLocation PalletLocation = Pallet->MovePlayerToInteractionPoint(D1Survivor);

	if (PalletLocation == EPalletLocation::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalletLocation None"));
		return;
	}

	FName SectionName;
	if (CurrentSpeed < 300.f) // 서있거나 걷고있을 때
	{
		SectionName = (PalletLocation == EPalletLocation::LT) ? "VaultPalletLT" : "VaultPalletRT";
	}
	else
	{
		SectionName = (PalletLocation == EPalletLocation::LT) ? "VaultPalletLTFast" : "VaultPalletRTFast";
	}

	// 애니메이션 실행
	if (CachedAnimInstance.IsValid())
	{
		SetCreatureState(ECreatureState::Parkour);

		D1Survivor->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
		D1Survivor->PlayAnimMontage(PalletMontage, 1.0f, SectionName);
	}
}
void AD1SurvivorController::EnableVaultAfterDrop()
{
	bCanVaultAfterDrop = true;
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
