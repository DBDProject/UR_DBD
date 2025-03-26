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
#include "Interactables/D1ExitGate.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Net/UnrealNetwork.h"
#include "Interactables/D1Hook.h"
#include "D1SurvivorSoundManager.h"
#include "Items/D1Medkit.h"
#include "Items/D1Toolbox.h"

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
	PlayerCameraManager->ViewPitchMin = -45.0f; // 최소 Pitch (아래 제한)
	PlayerCameraManager->ViewPitchMax = 45.0f;  // 최대 Pitch (위 제한)

	if (IsLocalController())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this; // 선택사항
		SpawnParams.Instigator = GetPawn(); // 선택사항

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FVector::ZeroVector);

		SoundManager = GetWorld()->SpawnActor<AD1SurvivorSoundManager>(
			SoundManagerClass,
			SpawnTransform,
			SpawnParams
		);
	}
}

void AD1SurvivorController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter()); // 컨트롤러가 새로운 캐릭터를 제어할 때 갱신
	if (D1Survivor.IsValid())
	{
		D1Survivor.Get()->GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	}
}

void AD1SurvivorController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ThisClass::Input_StartRun);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ThisClass::Input_StopRun);

		auto CrouchAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Crouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::Input_StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::Input_StopCrouch);

		// 좌클릭
		auto InteractAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Interact);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Input_StartInteract_LeftClick);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ThisClass::Input_StopInteract_LeftClick);

		// 스페이스
		auto InteractAction2 = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Parkour);
		EnhancedInputComponent->BindAction(InteractAction2, ETriggerEvent::Started, this, &ThisClass::Input_StartInteract_Space);
		EnhancedInputComponent->BindAction(InteractAction2, ETriggerEvent::Completed, this, &ThisClass::Input_StopInteract_Space);

		// 우클릭
		auto InteractAction3 = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_ItemUsage);
		EnhancedInputComponent->BindAction(InteractAction3, ETriggerEvent::Started, this, &ThisClass::Input_StartInteract_RightClick);
		EnhancedInputComponent->BindAction(InteractAction3, ETriggerEvent::Completed, this, &ThisClass::Input_StopInteract_RightClick);

		// (1번)
		auto PointToAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_PointTo);
		EnhancedInputComponent->BindAction(PointToAction, ETriggerEvent::Started, this, &ThisClass::Input_PointTo);
		// (2번)
		auto ComeHereAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_ComeHere);
		EnhancedInputComponent->BindAction(ComeHereAction, ETriggerEvent::Started, this, &ThisClass::Input_ComeHere);
	}
}

void AD1SurvivorController::Input_Move(const FInputActionValue& InputValue)
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());
	if (!D1Survivor.IsValid())	return;
	if (!(D1Survivor->GetSurvivorState() == ESurvivorState::Crawl) &&
		!(D1Survivor->GetSurvivorState() == ESurvivorState::Injured) &&
		!(D1Survivor->GetSurvivorState() == ESurvivorState::Healthy))
	{
		return;
	}
	if (D1Survivor->GetIsSelfRecovering() == true) return;

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

void AD1SurvivorController::Input_StartRun()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid() || !D1Survivor.Get()->GetSurvivoreSet()) return;
	if (!(D1Survivor->GetSurvivorState() == ESurvivorState::Injured) && !(D1Survivor->GetSurvivorState() == ESurvivorState::Healthy))
	{
		return;
	}

	if (IsLocalController()) // 로컬에서 즉시 실행
	{
		StartRun_Local();
	}

	Server_StartRun();

}

void AD1SurvivorController::Input_StopRun()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid() || !D1Survivor.Get()->GetSurvivoreSet()) return;

	if (IsLocalController()) // 로컬에서 즉시 실행
	{
		StopRun_Local();
	}

	Server_StopRun();

}

void AD1SurvivorController::Input_StartCrouch()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;
	if (!(D1Survivor->GetSurvivorState() == ESurvivorState::Injured) && !(D1Survivor->GetSurvivorState() == ESurvivorState::Healthy))
	{
		return;
	}
	if (GetCreatureState() == ECreatureState::Parkour) return;

	if (D1Survivor->GetCharacterMovement()->NavAgentProps.bCanCrouch == false)
		D1Survivor->GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SetCreatureState(ECreatureState::Crouch);
	D1Survivor.Get()->Crouch();

	if (HasAuthority())
	{
		SetCreatureState(ECreatureState::Crouch);
		D1Survivor.Get()->Crouch();
	}
}

void AD1SurvivorController::Input_StopCrouch()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;

	SetCreatureState(ECreatureState::None);
	D1Survivor.Get()->UnCrouch();

	if (HasAuthority())
	{
		SetCreatureState(ECreatureState::None);
		D1Survivor.Get()->UnCrouch();
	}
}

void AD1SurvivorController::Input_StartInteract_LeftClick()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;
	if (GetCreatureState() == ECreatureState::Parkour) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl)
	{
		if (!D1Survivor->HasAuthority())
		{
			D1Survivor->Server_SetSelfRecovering(true);
		}
		else
		{
			D1Survivor->SetIsSelfRecovering(true);
		}
		return;
	}

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Hooked
		&& D1Survivor->GetHookHealth() > 45.f)
	{
		D1Survivor->StartEscapeAttempt();
		return;
	}
	if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured || D1Survivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		// 발전기
		if (AD1Generator* Generator = Cast<AD1Generator>(D1Survivor.Get()->GetDetectedObject()))
		{
			if (Generator->GetIsRepairBlocked() == true) return;

			D1Survivor->StartRepair();
			return;
		}

		// 치유
		if (AD1SurvivorBase* TargetSurvivor = Cast<AD1SurvivorBase>(D1Survivor.Get()->GetDetectedObject()))
		{
			if (TargetSurvivor->GetSurvivorState() == ESurvivorState::Injured || TargetSurvivor->GetSurvivorState() == ESurvivorState::Crawl)
				if (IsLocalController())
				{
					StartHeal_Local(TargetSurvivor);
				}
			Server_StartHeal(TargetSurvivor);

			return;
		}
	}
}

void AD1SurvivorController::Input_StopInteract_LeftClick()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;


	if (D1Survivor->GetSurvivorState() == ESurvivorState::Crawl)
	{
		if (!D1Survivor->HasAuthority())
		{
			D1Survivor->Server_SetSelfRecovering(false);
		}
		else
		{
			D1Survivor->SetIsSelfRecovering(false);
		}
		return;
	}
	if (D1Survivor->GetSurvivorState() == ESurvivorState::Hooked
		&& D1Survivor->GetHookHealth() > 50.f)
	{
		D1Survivor->CancelEscapeAttempt();
	}

	if (AD1Generator* Generator = Cast<AD1Generator>(D1Survivor.Get()->GetDetectedObject()))
	{
		if (D1Survivor->GetIsFail() == true) return;

		D1Survivor->StopRepair();
		return;
	}
	if (AD1SurvivorBase* TargetSurvivor = Cast<AD1SurvivorBase>(D1Survivor.Get()->GetDetectedObject()))
	{
		if (IsLocalController())
		{
			StopHeal_Local(TargetSurvivor);
		}
		Server_StopHeal(TargetSurvivor);

		return;
	}
}

void AD1SurvivorController::Input_StartInteract_Space()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;

	if (GetCreatureState() == ECreatureState::Parkour) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured || D1Survivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		// 창 파쿠르
		if (AD1VaultObject* VaultTarget = Cast<AD1VaultObject>(D1Survivor.Get()->GetVaultTarget()))
		{
			// 현재 속도 가져오기
			float CurrentSpeed = D1Survivor.Get()->GetVelocity().Size();

			// 속도 기준으로 파쿠르 속도 결정
			EVaultType VaultType = EVaultType::Medium; // 기본값 Medium
			if (CurrentSpeed > 300.0f) VaultType = EVaultType::Fast;  // 달리기 속도
			else if (CurrentSpeed < 100.0f) VaultType = EVaultType::Slow; // 웅크리기 속도

			PerformVault(VaultType);

			return;
		}
		// 판자
		if (AD1Pallet* Pallet = Cast<AD1Pallet>(D1Survivor.Get()->GetCurrentPallet()))
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
		// 탈출구
		if (AD1ExitGate* Gate = Cast<AD1ExitGate>(D1Survivor.Get()->GetDetectedObject()))
		{
			StartExitOpening_Local();

			return;
		}
		// 갈고리 구출
		if (AD1SurvivorBase* TargetSurvivor = Cast<AD1SurvivorBase>(D1Survivor.Get()->GetDetectedObject()))
		{
			if (TargetSurvivor->GetSurvivorState() != ESurvivorState::Hooked)
				return;

			if (IsLocalController())
			{
				Rescue_Local(TargetSurvivor);
			}
			Server_Rescue(TargetSurvivor);

			return;
		}
	}
}

void AD1SurvivorController::Input_StopInteract_Space()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	// 탈출구
	if (AD1ExitGate* Gate = Cast<AD1ExitGate>(D1Survivor.Get()->GetDetectedObject()))
	{
		StopExitOpening_Local();

		return;
	}
}

void AD1SurvivorController::Input_StartInteract_RightClick()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor->UseCurrentItem();
	}
}

void AD1SurvivorController::Input_StopInteract_RightClick()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());

	if (!D1Survivor.IsValid()) return;

	D1Survivor->NotUseCurrentItem();
}

void AD1SurvivorController::Input_PointTo()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());
	if (!D1Survivor.IsValid()) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		D1Survivor->PlayMontage(D1Survivor->GestureMontage, "PointTo");
	}

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor->PlayMontage(D1Survivor->GestureMontage, "Injured_PointTo");
	}
}

void AD1SurvivorController::Input_ComeHere()
{
	D1Survivor = Cast<AD1SurvivorBase>(GetCharacter());
	if (!D1Survivor.IsValid()) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Healthy)
	{
		D1Survivor->PlayMontage(D1Survivor->GestureMontage, "ComeHere");
	}

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor->PlayMontage(D1Survivor->GestureMontage, "Injured_ComeHere");
	}
}

void AD1SurvivorController::RepairDelegate_Start()
{
	if (IsLocalPlayerController())
		RepaireStartDelegate.Broadcast();
}

void AD1SurvivorController::RepairDelegate_End()
{
	if (IsLocalPlayerController())
		RepaireEndDelegate.Broadcast();
}

void AD1SurvivorController::StartRun_Local()
{

	if (!D1Survivor.IsValid()) return;

	if (D1Survivor.Get()->GetSurvivorState() == ESurvivorState::Healthy)
	{
		D1Survivor.Get()->GetCharacterMovement()->MaxWalkSpeed = D1Survivor.Get()->GetSurvivoreSet()->GetRunSpeed();
	}
	else if (D1Survivor.Get()->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor.Get()->GetCharacterMovement()->MaxWalkSpeed = D1Survivor.Get()->GetSurvivoreSet()->GetInjRunSpeed();
	}
}

void AD1SurvivorController::StopRun_Local()
{
	if (!D1Survivor.IsValid()) return;

	SetCreatureState(ECreatureState::None);

	if (D1Survivor.Get()->GetSurvivorState() == ESurvivorState::Healthy)
	{
		D1Survivor.Get()->GetCharacterMovement()->MaxWalkSpeed = D1Survivor.Get()->GetSurvivoreSet()->GetWalkSpeed();
	}
	else if (D1Survivor.Get()->GetSurvivorState() == ESurvivorState::Injured)
	{
		D1Survivor.Get()->GetCharacterMovement()->MaxWalkSpeed = D1Survivor.Get()->GetSurvivoreSet()->GetInjWalkSpeed();
	}
}

void AD1SurvivorController::Server_StartRun_Implementation()
{
	if (HasAuthority())
	{
		Multi_StartRun();
	}
}

void AD1SurvivorController::Server_StopRun_Implementation()
{
	if (HasAuthority())
	{
		Multi_StopRun();
	}
}

void AD1SurvivorController::Multi_StartRun_Implementation()
{
	if (IsLocalController()) return;

	StartRun_Local();
}

void AD1SurvivorController::Multi_StopRun_Implementation()
{
	if (IsLocalController()) return;

	StopRun_Local();
}

void AD1SurvivorController::StartHeal_Local(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor.IsValid() || !TargetSurvivor) return;

	if (TargetSurvivor->GetSurvivorState() == ESurvivorState::Healthy || !TargetSurvivor->GetCanBeHealed())
	{
		StopHeal_Local(TargetSurvivor);
		return;
	}

	// 플레이어 방향 조정
	FRotator LookAtRotation = (TargetSurvivor->GetActorLocation() - D1Survivor->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;
	D1Survivor->SetActorRotation(LookAtRotation);

	D1Survivor->SetHealingTargetState(TargetSurvivor->GetSurvivorState());
	D1Survivor->GetCharacterMovement()->DisableMovement();

	TargetSurvivor->BeingHealing(D1Survivor.Get());
}

void AD1SurvivorController::StopHeal_Local(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor.IsValid() || !TargetSurvivor) return;

	D1Survivor->SetIsHealing(false);
	D1Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	D1Survivor->SetHealingTargetState(ESurvivorState::None);

	TargetSurvivor->StopBeingHealing();

}

void AD1SurvivorController::Server_StartHeal_Implementation(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor.IsValid() || !TargetSurvivor) return;

	Multicast_StartHeal(TargetSurvivor);
}

void AD1SurvivorController::Server_StopHeal_Implementation(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor.IsValid() || !TargetSurvivor) return;

	Multicast_StopHeal(TargetSurvivor);
}


void AD1SurvivorController::Multicast_StartHeal_Implementation(AD1SurvivorBase* TargetSurvivor)
{
	StartHeal_Local(TargetSurvivor);
}

void AD1SurvivorController::Multicast_StopHeal_Implementation(AD1SurvivorBase* TargetSurvivor)
{
	StopHeal_Local(TargetSurvivor);
}

void AD1SurvivorController::StartExitOpening_Local()
{
	if (!D1Survivor.IsValid()) return;

	// 감지된 오브젝트를 ExitGate로 캐스팅
	if (AD1ExitGate* Gate = Cast<AD1ExitGate>(D1Survivor->GetDetectedObject()))
	{
		// (로컬)플레이어 위치 이동
		if (IsLocalPlayerController())
			D1Survivor->MoveToExitGateStartPosition(Gate);

		// 서버에 검증 요청
		Server_StartExitOpening(Gate);
	}
}

void AD1SurvivorController::StopExitOpening_Local()
{
	if (!D1Survivor.IsValid()) return;

	// 감지된 오브젝트를 ExitGate로 캐스팅
	if (AD1ExitGate* Gate = Cast<AD1ExitGate>(D1Survivor->GetDetectedObject()))
	{
		// 서버에 검증 요청
		Server_StopExitOpening(Gate);
	}
}

void AD1SurvivorController::Server_StartExitOpening_Implementation(AD1ExitGate* Gate)
{
	// 서버에서만 실행되도록 보장
	if (!HasAuthority() || !Gate) return;

	// 서버에서 검증
	if (Gate->GetCrrentState() != EGateState::Closed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] 탈출구가 이미 열려 있음!"));
		return;
	}

	if (!Gate->GetActivateExitGate())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] 아직 탈출구가 활성화되지 않음!"));
		return;
	}

	// 검증 통과 후 클라이언트에게 탈출구 열기 요청
	Multicast_StartExitOpening(Gate);
}

void AD1SurvivorController::Server_StopExitOpening_Implementation(AD1ExitGate* Gate)
{
	// 서버에서만 실행되도록 보장
	if (!HasAuthority() || !Gate) return;

	// 서버에서 검증
	if (!Gate->GetActivateExitGate())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] 아직 탈출구가 활성화되지 않음!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Server] StopOpening 호출됨"));

	// 검증 통과 후 클라이언트에게 탈출구 닫기 요청
	Multicast_StopExitOpening(Gate);
}

void AD1SurvivorController::Multicast_StartExitOpening_Implementation(AD1ExitGate* Gate)
{
	if (!Gate || !D1Survivor.IsValid()) return;

	// 이동 입력 차단
	D1Survivor->GetCharacterMovement()->DisableMovement();
	D1Survivor->GetCharacterMovement()->StopMovementImmediately();

	// 플레이어 위치 이동
	D1Survivor->MoveToExitGateStartPosition(Gate);
	// 탈출구 열기 시작
	Gate->StartOpening(D1Survivor.Get());
}


void AD1SurvivorController::Multicast_StopExitOpening_Implementation(AD1ExitGate* Gate)
{
	if (!Gate || !D1Survivor.IsValid()) return;

	// 이동 가능하게 변경
	D1Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	D1Survivor->SetIsExitGateOpening(false);
	// 탈출구 닫기 실행
	Gate->StopOpening();
}

void AD1SurvivorController::PerformVault(EVaultType VaultType)
{
	if (!D1Survivor.IsValid() || !D1Survivor->GetVaultTarget()) return;
	if (!D1Survivor->VaultMontage) return;

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

	D1Survivor->PlayMontage(D1Survivor->VaultMontage, SectionName);
}

void AD1SurvivorController::DropPallet()
{
	AD1Pallet* Pallet = D1Survivor->GetCurrentPallet();
	if (!D1Survivor.IsValid() || !Pallet) return;

	if (Pallet->GetCurrentState() == EPalletState::Down) return;

	// 현재 속도 가져오기
	float CurrentSpeed = D1Survivor->GetVelocity().Size();

	// 플레이어 위치, 방향 이동
	EPalletLocation PalletLocation = Pallet->FindClosestInteractionPoint(D1Survivor.Get());
	D1Survivor->GetCurrentPallet()->SetCurrentLocation(PalletLocation);
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
	D1Survivor->PlayMontage(D1Survivor->PalletMontage, SectionName);
	Pallet->StartDropping(D1Survivor.Get());

	bCanVaultAfterDrop = false;
	GetWorld()->GetTimerManager().SetTimer(VaultCooldownTimer, this, &AD1SurvivorController::EnableVaultAfterDrop, 0.5f, false);

	UE_LOG(LogTemp, Warning, TEXT("Pallet Drop"));
}

void AD1SurvivorController::VaultPallet()
{
	AD1Pallet* Pallet = D1Survivor->GetCurrentPallet();
	if (!D1Survivor.IsValid() || !Pallet) return;

	if (Pallet->GetCurrentState() == EPalletState::Up) return;


	// 현재 속도 가져오기
	float CurrentSpeed = D1Survivor->GetVelocity().Size();

	// 플레이어 위치, 방향 이동
	EPalletLocation PalletLocation = Pallet->FindClosestInteractionPoint(D1Survivor.Get());
	D1Survivor->GetCurrentPallet()->SetCurrentLocation(PalletLocation);

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

	D1Survivor->PlayMontage(D1Survivor->PalletMontage, SectionName);

}

void AD1SurvivorController::Rescue_Local(AD1SurvivorBase* TargetSurvivor)
{
	if (!D1Survivor.IsValid() || !TargetSurvivor) return;

	if (TargetSurvivor->GetSurvivorState() != ESurvivorState::Hooked)	return;

	// 구출자 위치 조정
	FVector RescueLocation = TargetSurvivor->GetActorLocation() + FVector(60.f, 0.f, 0.f);
	D1Survivor->SetActorLocation(RescueLocation);
	// 구출자 방향 조정
	FRotator LookAtRotation = (TargetSurvivor->GetActorLocation() - D1Survivor->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;
	D1Survivor->SetActorRotation(LookAtRotation);

	D1Survivor->PlayMontage(D1Survivor->RescueMontage, "Rescue");
	TargetSurvivor->PlayMontage(D1Survivor->RescueMontage, "BeingRescued");
}

void AD1SurvivorController::Server_Rescue_Implementation(AD1SurvivorBase* TargetSurvivor)
{
	if (HasAuthority())
		Multicast_Rescue(TargetSurvivor);
}

void AD1SurvivorController::Multicast_Rescue_Implementation(AD1SurvivorBase* TargetSurvivor)
{
	Rescue_Local(TargetSurvivor);
}

void AD1SurvivorController::EnableVaultAfterDrop()
{
	bCanVaultAfterDrop = true;
}

void AD1SurvivorController::MoveToGeneratorPosition(EGeneratorInteractionPosition Position)
{
	if (!D1Survivor.IsValid() || !D1Survivor->GetCurrentGenerator()) return;

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
		TargetLocation = GeneratorLocation - ForwardVector * 105.f;
		break;
	case EGeneratorInteractionPosition::Left:
		TargetLocation = GeneratorLocation - RightVector * 80.f;
		break;
	case EGeneratorInteractionPosition::Right:
		TargetLocation = GeneratorLocation + RightVector * 82.5f;
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

void AD1SurvivorController::SetCreatureState(ECreatureState InState)
{
	if (D1Survivor.IsValid())
	{
		D1Survivor->CreatureState = InState;
	}
}

ECreatureState AD1SurvivorController::GetCreatureState()
{
	return D1Survivor.IsValid() ? D1Survivor->CreatureState : ECreatureState::None;
}

// 사운드
void AD1SurvivorController::Client_UpdateBGMLevel_Implementation(EBGMLevel NewLevel)
{
	if (!D1Survivor.IsValid()) return;

	if (D1Survivor->GetSurvivorState() == ESurvivorState::Healthy ||
		D1Survivor->GetSurvivorState() == ESurvivorState::Injured)
	{
		if (SoundManager)
		{
			// 상태 비교 후 전환
			if (CurrentBGMLevel != NewLevel)
			{
				CurrentBGMLevel = NewLevel;

				switch (NewLevel)
				{
				case EBGMLevel::Normal:  SoundManager->PlayBGM(NormalBGM, 2.5f); break;
				case EBGMLevel::Warning: SoundManager->PlayBGM(WarningBGM, 1.5f); break;
				case EBGMLevel::Threat:  SoundManager->PlayBGM(ThreatBGM, 1.0f); break;
				case EBGMLevel::Terror:  SoundManager->PlayBGM(TerrorBGM, 0.5f); break;
				default: break;
				}
			}
		}
	}
}

void AD1SurvivorController::Client_PlaySurvivorBGMByLevel_Implementation(EBGMLevel NewLevel)
{
	PlaySurvivorBGMByLevel(NewLevel);
}
void AD1SurvivorController::PlaySurvivorBGMByLevel(EBGMLevel NewLevel)
{
	if (HasAuthority())
	{
		Client_PlaySurvivorBGMByLevel(NewLevel);
	}
	if (!IsLocalPlayerController())	return;

	if (!SoundManager || !D1Survivor.IsValid()) return;
	if (CurrentBGMLevel == NewLevel)	return;

	CurrentBGMLevel = NewLevel;

	switch (NewLevel)
	{
	case EBGMLevel::Normal:
		SoundManager->PlayBGM(NormalBGM, 2.5f); break;
	case EBGMLevel::Warning:
		SoundManager->PlayBGM(WarningBGM, 1.5f); break;
	case EBGMLevel::Threat:
		SoundManager->PlayBGM(ThreatBGM, 1.0f); break;
	case EBGMLevel::Terror:
		SoundManager->PlayBGM(TerrorBGM, 0.5f); break;
	case EBGMLevel::Crawl:
		SoundManager->PlayBGM(CrawlBGM, 1.0f); break;
	case EBGMLevel::HookPart1:
		SoundManager->PlayBGM(HookBGM_Part1, 1.0f); break;
	case EBGMLevel::HookPart2:
		SoundManager->PlayBGM(HookBGM_Part2, 1.0f); break;
	default:
		break;
	}
}

void AD1SurvivorController::StopSurvivorBGM(float FadeTime)
{
	if (!SoundManager) return;

	SoundManager->StopBGM(FadeTime);
}