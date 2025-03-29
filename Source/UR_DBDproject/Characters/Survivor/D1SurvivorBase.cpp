// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Survivor/D1SurvivorBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "D1SurvivorState.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"
#include "Interactables/D1Generator.h"
#include "Components/BoxComponent.h"
#include "Interactables/D1VaultObject.h"
#include "Interactables/D1Pallet.h"
#include "Interactables/D1ExitArea.h"
#include "Animation/D1SurvivorBaseAnim.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Interactables/D1Hook.h"
#include "Interactables/D1ExitGate.h"
#include "Interactables/D1ScentSphere.h"
#include "Items/D1ItemBase.h"
#include "Items/D1Medkit.h"
#include "Items/D1Toolbox.h"
#include "Net/UnrealNetwork.h"
#include "D1SurvivorController.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "System/D1GameState.h"

AD1SurvivorBase::AD1SurvivorBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	bReplicates = true;

	UE_LOG(LogTemp, Warning, TEXT("생존자 생성됨! %s"), *GetName());
	auto a = this;
	GetCharacterMovement()->bOrientRotationToMovement = false;			// 이동 방향을 자동으로 바라보지 않음
	bUseControllerRotationYaw = false;									// 컨트롤러의 방향을 따라 캐릭터가 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 650.f, 0.f);	// 회전 속도


	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->bUsePawnControlRotation = true;				// 플레이어가 아니라 컨트롤러 회전을 따름

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false; // 카메라 독립적으로 회전 가능

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetWalkableFloorAngle(50.f); // 기본 44 -> 60으로 증가

	// 상호작용 감지용 박스 컴포넌트 (상호작용 범위를 넓게 설정)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollider"));
	InteractionBox->SetupAttachment(GetCapsuleComponent());
	InteractionBox->SetBoxExtent(FVector(50.f, 50.f, 100.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	RootComponent->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UAnimMontage> VaultMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_Vault.AM_Meg_Vault"));
	if (VaultMontageAsset.Succeeded())
	{
		VaultMontage = VaultMontageAsset.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> PalletMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_Pallet.AM_Meg_Pallet"));
	if (PalletMontageAsset.Succeeded())
	{
		PalletMontage = PalletMontageAsset.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_Hit.AM_Meg_Hit"));
	if (HitMontageAsset.Succeeded())
	{
		HitMontage = HitMontageAsset.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RescueMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_Rescue.AM_Meg_Rescue"));
	if (RescueMontageAsset.Succeeded())
	{
		RescueMontage = RescueMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EscapeMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_AttemptEscape.AM_Meg_AttemptEscape"));
	if (EscapeMontageAsset.Succeeded())
	{
		EscapeMontage = EscapeMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> SpiderMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_Entity.AM_Meg_Entity"));
	if (SpiderMontageAsset.Succeeded())
	{
		SpiderMontage = SpiderMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> GeneratorMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_Generator_Fail.AM_Meg_Generator_Fail"));
	if (GeneratorMontageAsset.Succeeded())
	{
		S_GeneratorMontage = GeneratorMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> GestureMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Gesture.AM_Gesture"));
	if (GestureMontageAsset.Succeeded())
	{
		GestureMontage = GestureMontageAsset.Object;
	}
}

void AD1SurvivorBase::BeginPlay()
{
	Super::BeginPlay();

	// TEST
	//CurrentState = ESurvivorState::Crawl;
	//HealingProgress = 90.f;

	// 컨트롤러의 기본 회전값을 설정하여 카메라 방향 조정
	//if (Controller)
	//{
	//	Controller->SetControlRotation(FRotator(-45, 0, 0));
	//}

	if (InteractionBox)
	{
		// 콜리전 이벤트 바인딩
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AD1SurvivorBase::OnOverlapBegin);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AD1SurvivorBase::OnOverlapEnd);
	}

	// Temp
	//EquipItem(BP_ToolboxClass);
}

void AD1SurvivorBase::InitAbilitySystem()
{
	if (HasAuthority())
	{
		if (AD1SurvivorState* PS = GetPlayerState<AD1SurvivorState>())
		{
			AbilitySystemComponent = Cast<UD1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
			if (!AbilitySystemComponent) return;

			AbilitySystemComponent->InitAbilityActorInfo(PS, this);

			AttributeSet = PS->GetD1SurvivorSet();
			SurvivorSet = Cast<UD1SurvivorSet>(AttributeSet);
			if (SurvivorSet)
			{
				GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetWalkSpeed();
				GetCharacterMovement()->MaxWalkSpeedCrouched = SurvivorSet->GetCrouchSpeed();
			}
		}
	}
}

void AD1SurvivorBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystem();

	if (HasAuthority())
	{
		EquipItem(BP_MedkitClass);
	}
}

void AD1SurvivorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1SurvivorBase, SurvivorSet);
	DOREPLIFETIME(AD1SurvivorBase, EquippedItem);
	DOREPLIFETIME(AD1SurvivorBase, bIsRepairing);
	DOREPLIFETIME(AD1SurvivorBase, bPrevRepairing);
	DOREPLIFETIME(AD1SurvivorBase, PrevState);
	DOREPLIFETIME(AD1SurvivorBase, InteractionPosition);
	DOREPLIFETIME(AD1SurvivorBase, bIsFail);
	DOREPLIFETIME(AD1SurvivorBase, bIsHealing);
	DOREPLIFETIME(AD1SurvivorBase, HealingTargetState);
	DOREPLIFETIME(AD1SurvivorBase, HealingProgress);
	DOREPLIFETIME(AD1SurvivorBase, bIsBeingHealed);
	DOREPLIFETIME(AD1SurvivorBase, bCanBeHealed);
	DOREPLIFETIME(AD1SurvivorBase, CurrentState);
	DOREPLIFETIME(AD1SurvivorBase, bIsExitGateOpening);
	DOREPLIFETIME(AD1SurvivorBase, CrawlHealth);
	DOREPLIFETIME(AD1SurvivorBase, bIsCrawlSelfRecovering);
	DOREPLIFETIME(AD1SurvivorBase, bIsCarryHook);
	DOREPLIFETIME(AD1SurvivorBase, HookedCount);
	DOREPLIFETIME(AD1SurvivorBase, HookHealth);
	DOREPLIFETIME(AD1SurvivorBase, bIsHookSkillCheckEnable);
	DOREPLIFETIME(AD1SurvivorBase, bIsHookSkillCheckFail);
	DOREPLIFETIME(AD1SurvivorBase, EscapeGauge);
	DOREPLIFETIME(AD1SurvivorBase, PlayerIndex);
	DOREPLIFETIME(AD1SurvivorBase, bIsUsingMedkit);
}

void AD1SurvivorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EquippedItem)
	{
		if (HasAuthority()) // 박쥐 폼일 때 초기화 못하게
		{
			if (GetKiller() &&
				CachedKiller->GetCurrentTransformState() == EDraculaTransformationState::Bat)
			{
				return;
			}
		}

		const bool bShouldShow = ShouldShowItemMesh();
		if (EquippedItem->GetIsVisible() != bShouldShow)
		{
			if (EquippedItem->GetIsVisible() == true)
			{
				EquippedItem->DeactivateItem();
			}
			else if (EquippedItem->GetIsVisible() == false)
			{
				EquippedItem->ActivateItem();
			}
		}
	}

	if (bPlayingIntro)
	{
		if (GetController()->IsLocalPlayerController())
		{
			CurrentAngle -= OrbitSpeed * DeltaTime;
			SpringArm->SetRelativeRotation(FRotator(0.f, CurrentAngle, 0.f));
		}
	}

	if (bPlayingEscape)
	{
		if (GetController()->IsLocalPlayerController())
		{
			CurrentAngle += OrbitSpeed * DeltaTime;
			CurrentArmLength += ArmLengthSpeed * DeltaTime;

			if (CurrentAngle <= 150.f)
				SpringArm->SetRelativeRotation(FRotator(0.f, -CurrentAngle, 0.f));

			if (CurrentArmLength <= 450.f)
				SpringArm->TargetArmLength = CurrentArmLength;

			AddMovementInput(ExitAreaFowardVector, 1.f, true);
		}
		return;
	}

	SmoothCameraTransition(DeltaTime);

	if (!HasAuthority()) return; // 서버에서만 실행

	if (bIsBeingHealed || bIsCrawlSelfRecovering || bIsUsingMedkit)
	{
		UpdateHealingProgress(DeltaTime);
	}

	if (CurrentState == ESurvivorState::Crawl)
	{
		UpdateCrawlBleedOut(DeltaTime);
	}

	if (CurrentState == ESurvivorState::Hooked)
	{
		UpdateHookBleedOut(DeltaTime);
	}

	if (WolfCheck)
	{
		if (GetKiller() &&
			CachedKiller->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
		{
			GetWorldTimerManager().SetTimer(
				WolfCheckTimerHandle,
				this,
				&AD1SurvivorBase::TrySpawnScentSphere,
				5.0f,
				true
			);
			WolfCheck = false;
		}
	}

}
// 웅크릴 때 카메라 보간
void AD1SurvivorBase::SmoothCameraTransition(float DeltaTime)
{
	if (!SpringArm) return;

	float DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float CrouchCapsuleHalfHeight = GetCharacterMovement()->CrouchedHalfHeight;

	// 카메라 보간 속도
	float CameraLerpSpeed = 6.f;

	float TargetHeight = bIsCrouched ? CrouchCapsuleHalfHeight * 0.7f : DefaultCapsuleHalfHeight * 0.7f;
	float CurrentHeight = SpringArm->SocketOffset.Z;

	float NewHeight = FMath::FInterpTo(CurrentHeight, TargetHeight, DeltaTime, CameraLerpSpeed);
	SpringArm->SocketOffset = FVector(0.f, 0.f, NewHeight);
}

void AD1SurvivorBase::UpdateHealingProgress(float DeltaTime)
{
	if (!bIsBeingHealed && !bIsCrawlSelfRecovering && !bIsUsingMedkit) return;

	float CurrentHealingRate = bIsBeingHealed ? HealingRate : SelfRecoveryRate;

	// 치료 진행도 증가
	HealingProgress += CurrentHealingRate * DeltaTime;
	HealingProgress = FMath::Clamp(HealingProgress, 0.0f, bIsCrawlSelfRecovering ? 95.0f : 100.0f);

	//UE_LOG(LogTemp, Warning, TEXT("치료 진행도: %.2f%%"), HealingProgress);

	// 치료가 완료되었는지 확인
	if (HealingProgress >= 100.0f)
	{
		FinishHealing();
	}
}

void AD1SurvivorBase::UpdateCrawlBleedOut(float DeltaTime)
{
	if (CurrentState != ESurvivorState::Crawl) return;

	CrawlHealth -= BleedOutRate * DeltaTime;
	CrawlHealth = FMath::Clamp(CrawlHealth, 0.0f, 100.0f);

	//UE_LOG(LogTemp, Warning, TEXT("[출혈] HP: %.2f%%"), CrawlHealth);

	if (CrawlHealth <= 0.f)
	{
		// TEMP
		DieFromBleedOut();
	}
}

void AD1SurvivorBase::UpdateHookBleedOut(float DeltaTime)
{
	if (CurrentState != ESurvivorState::Hooked) return;
	//HookHealth -= HookBleedOutRate * DeltaTime*6;
	HookHealth -= HookBleedOutRate * DeltaTime;
	HookHealth = FMath::Clamp(HookHealth, 0.0f, 100.0f);

	//UE_LOG(LogTemp, Warning, TEXT("[갈고리][출혈] HP: %.2f%%"), HookHealth);

	if (HookHealth <= 50.f && CurrentHook.IsValid())
	{
		if (CurrentHook->GetEntityVisible() == false)
		{
			Multicast_StartEntityEvent(this);
		}
	}
	if (HookHealth <= 45.f && bIsHookEventReaction == false)
	{
		if (CurrentHook.IsValid())
		{
			bIsHookEventReaction = true;
			Multicast_StartEntityReaction();
		}
	}

	if (HookHealth <= 0.f || HookedCount >= 3)
	{
		DieFromEntity();
		return;
	}


	if (bIsHookSkillCheckEnable == true
		&& bIsHookEventSkillCheck == false)
	{
		Multicast_SkillCheckEnable(true);
		bIsHookEventSkillCheck = true;
	}
}

void AD1SurvivorBase::MoveToGeneratorPosition(EGeneratorInteractionPosition Position)
{
	if (!CurrentGenerator.IsValid()) return;

	FVector GeneratorLocation = CurrentGenerator->GetActorLocation();
	FVector ForwardVector = CurrentGenerator->GetActorForwardVector();
	FVector RightVector = CurrentGenerator->GetActorRightVector();
	FVector TargetLocation;

	// 플레이어를 발전기 위치로 이동
	switch (Position)
	{
	case EGeneratorInteractionPosition::Front:
		TargetLocation = GeneratorLocation + ForwardVector * 100.f;
		break;
	case EGeneratorInteractionPosition::Back:
		TargetLocation = GeneratorLocation - ForwardVector * 110.f;
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

	SetActorLocation(TargetLocation);

	// 플레이어 방향을 발전기로 조정 (자동 회전)
	FRotator LookAtRotation = (GeneratorLocation - TargetLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지

	bUseControllerRotationYaw = true;
	SetActorRotation(LookAtRotation);
	bUseControllerRotationYaw = false;
}

void AD1SurvivorBase::StartRepair()
{
	AD1SurvivorController* PC = Cast<AD1SurvivorController>(GetController());
	if (PC)
	{
		if (PC->IsLocalController())
		{
			if (!GetCurrentGenerator()) return;

			if (GetCurrentGenerator()->GetIsRepairBlocked() ||
				GetCurrentGenerator()->GetIsCompleteRepair() ||
				GetCurrentGenerator()->GetRepairProgress() >= 100.f)
				return;

			EGeneratorInteractionPosition Position = GetCurrentGenerator()->FindInteractionPosition(this);

			MoveToGeneratorPosition(Position);
			SetInteractionPosition(Position);

			Server_StartRepair(Position);
		}
	}
}

void AD1SurvivorBase::Server_StartRepair_Implementation(EGeneratorInteractionPosition Position)
{
	MoveToGeneratorPosition(Position);
	SetInteractionPosition(Position);
	GetCurrentGenerator()->StartRepair(this, Position);
}

void AD1SurvivorBase::StopRepair()
{
	if (GetController())
	{
		if (GetController()->IsLocalController())
		{
			StopRepair_Local();
		}
	}
	Server_StopRepair();
}

void AD1SurvivorBase::Server_RequestSkillCheckSuccess_Implementation(AD1Generator* Generator)
{
	if (Generator)
	{
		Generator->OnSkillCheckSuccess(this);
	}
}

void AD1SurvivorBase::Server_RequestSkillCheckFail_Implementation(AD1Generator* Generator)
{
	if (Generator)
	{
		Generator->OnSkillCheckFail(this);
	}
}

void AD1SurvivorBase::Multicast_StartEntityGeneratorEvent_Implementation()
{
	if (AD1Generator* Generator = GetCurrentGenerator())
		Generator->StartDissolveEffect();
}

void AD1SurvivorBase::Multicast_StopEntityGeneratorEvent_Implementation()
{
	if (AD1Generator* Generator = GetCurrentGenerator())
	{
		if (Generator->GetEntityVisible() != true) return;

		Generator->StartDissolveDisappearEffect();
	}
}

void AD1SurvivorBase::StopRepair_Local()
{
	SetIsRepairing(false);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (!GetCurrentGenerator()) return;

	GetCurrentGenerator()->StopRepair(this);
	if (GetController())
	{
		if (GetController()->IsLocalPlayerController())
		{
			if (AD1SurvivorController* PC = Cast<AD1SurvivorController>(GetController()))
			{
				PC->RepairDelegate_End();
			}
		}
	}
}

void AD1SurvivorBase::Server_StopRepair_Implementation()
{
	Multi_StopRepair();
}

void AD1SurvivorBase::Multi_StopRepair_Implementation()
{
	StopRepair_Local();
}

void AD1SurvivorBase::MoveToVaultStartPosition()
{
	if (!VaultTarget.IsValid())	return;

	FVector VaultCenter = VaultTarget->GetActorLocation();
	FVector ObstacleNormal = VaultTarget->GetActorForwardVector();
	FVector SurvivorLocation = GetActorLocation();

	float OffsetDistance = -50.f; // 장애물 중앙 기준 앞으로 이동할 거리

	// 플레이어가 장애물 기준 앞쪽인지 뒤쪽인지 판단
	FVector ToObstacle = (VaultCenter - SurvivorLocation).GetSafeNormal();
	float Dot = FVector::DotProduct(ToObstacle, ObstacleNormal);

	if (Dot > 0)
	{
		ObstacleNormal *= -1; // 방향 반전
	}

	// 장애물 방향으로 이동
	FVector TargetLocation = VaultCenter - (ObstacleNormal * OffsetDistance);
	TargetLocation.Z += 88.f;  // 플레이어 위치 보정
	SetActorLocation(TargetLocation);

	// 장애물 방향으로 회전
	FRotator LookAtRotation = ObstacleNormal.Rotation();
	LookAtRotation.Yaw += 180.f; // 장애물 좌표값 보정

	bUseControllerRotationYaw = true;
	SetActorRotation(LookAtRotation);
	bUseControllerRotationYaw = false;

}

void AD1SurvivorBase::PlayMontage(UAnimMontage* Montage, FName SectionName = "Default")
{
	if (!Montage) return;

	if (GetController())
	{
		if (GetController()->IsLocalController())
		{
			PlayMontage_Local(Montage, SectionName);
		}
	}
	Server_PlayMontage(Montage, SectionName);
}


void AD1SurvivorBase::PlayMontage_Local(UAnimMontage* Montage, FName SectionName)
{
	if (Montage == VaultMontage)
	{
		CreatureState = ECreatureState::Parkour;

		MoveToVaultStartPosition();
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);

		PlayAnimMontage(Montage, 1.0f, SectionName);
	}
	else if (Montage == PalletMontage)
	{
		if (!CurrentPallet.IsValid())	return;

		if (CurrentPallet->GetCurrentState() == EPalletState::Up)
		{
			MovePlayerToPalletPoint();
			PlayAnimMontage(Montage, 1.0f, SectionName);
			CurrentPallet->SetCurrentState(EPalletState::Down);
		}
		else
		{
			MovePlayerToPalletPoint();
			CreatureState = ECreatureState::Parkour;
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);

			PlayAnimMontage(Montage, 1.0f, SectionName);
		}

	}
	else if (Montage == HitMontage)
	{
		if (CurrentState == ESurvivorState::Healthy)
		{
			GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetInjWalkSpeed();
			PlayAnimMontage(HitMontage, 1.0f, SectionName);
		}
		else if (CurrentState == ESurvivorState::Injured)
		{
			GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetCrawlSpeed();
			PlayAnimMontage(HitMontage, 1.0f, SectionName);

			// 상태 초기화
			HealingProgress = 0.f;
		}
	}

	else if (Montage == RescueMontage)
	{
		if (SectionName == "Rescue")
		{
			PlayAnimMontage(RescueMontage, 1.0f, SectionName);
		}
		else if (SectionName == "BeingRescued")
		{
			PlayAnimMontage(RescueMontage, 1.0f, SectionName);
		}
	}

	else if (Montage == EscapeMontage)
	{
		if (SectionName == "CancelAttempt")
		{
			StopAnimMontage(EscapeMontage);
		}
		else
		{
			PlayAnimMontage(EscapeMontage, 1.0f, SectionName);
		}
	}

	else
	{
		PlayAnimMontage(Montage, 1.0f, SectionName);
	}
}
void AD1SurvivorBase::Server_PlayMontage_Implementation(UAnimMontage* Montage, FName SectionName)
{
	Multicast_PlayMontage(Montage, SectionName);
}
void AD1SurvivorBase::Multicast_PlayMontage_Implementation(UAnimMontage* Montage, FName SectionName)
{
	PlayMontage_Local(Montage, SectionName);
}

void AD1SurvivorBase::Multicast_StartEntityEvent_Implementation(AD1SurvivorBase* Player)
{
	if (CurrentHook.IsValid())
		CurrentHook->StartDissolveEffect(Player);
}

void AD1SurvivorBase::Multicast_StopEntityEvent_Implementation()
{
	if (AD1Hook* Hook = GetCurrentHook())
	{
		if (Hook->GetEntityVisible() != true) return;

		Hook->StartDissolveDisappearEffect();
	}
}

void AD1SurvivorBase::Multicast_StartEntityReaction_Implementation()
{
	if (CurrentHook.IsValid())
	{
		PlayAnimMontage(SpiderMontage, 1.0f, "Reaction");
		CurrentHook->PlayEntityMontage("Reaction");
	}
}


void AD1SurvivorBase::StartOnHooked(AD1Hook* Hook)
{
	if (!Hook) return;
	if (CurrentState == ESurvivorState::Hooked) return;

	SetSurvivorState(ESurvivorState::Hooked);

	if (HasAuthority())
	{
		Multicast_AttachToHook(Hook);
	}
}

void AD1SurvivorBase::Multicast_AttachToHook_Implementation(AD1Hook* Hook)
{
	if (!Hook) return;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	FName HookSocket = "socket_SurvivorHook";
	AttachToComponent(Hook->GetHookMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		HookSocket);

	if (Hook)
		CurrentHook = Hook;
	CurrentHook->SetIsHooked(true);
	CurrentHook->SetInteractingPlayer(this);
	// 충돌 활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (AD1SurvivorController* PC = Cast<AD1SurvivorController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->PlaySurvivorBGMByLevel(EBGMLevel::HookPart1);
	}

	if (HasAuthority())
	{
		OnHooked();
	}
}

void AD1SurvivorBase::OnHooked()
{
	HookedCount++;
	//HookedCount = 2;
	bIsCarryHook = true;

	UE_LOG(LogTemp, Warning, TEXT("HookedCount : %d"), HookedCount)

		// 2번째 갈고리
		if (HookedCount == 2)
		{
			if (HookHealth > 50.f)
				HookHealth = 50.0f;
		}
}

void AD1SurvivorBase::OnHookSkillCheckFail()
{
	if (!CurrentHook.IsValid()) return;

	if (!HasAuthority())
	{
		Server_OnHookSkillCheckFail();
		return;
	}
}

void AD1SurvivorBase::Server_OnHookSkillCheckFail_Implementation()
{
	HookHealth -= 2.5f;
	Multicast_OnHookSkillCheckFail();
}

void AD1SurvivorBase::Multicast_OnHookSkillCheckFail_Implementation()
{
	bIsHookSkillCheckFail = true;
	CurrentHook->SetIsSkillCheckFail(true);
	UE_LOG(LogTemp, Warning, TEXT("스킬 체크 실패!"));
}

void AD1SurvivorBase::StartEscapeAttempt()
{
	if (!HasAuthority())
	{
		Server_StartEscapeAttempt();
		return;
	}

	// 탈출 애니메이션 실행 & 재생 시간 가져오기
	if (EscapeMontage)
	{
		PlayMontage(EscapeMontage, "Attempt");
		MaxEscapeGauge = EscapeMontage->GetPlayLength(); // 애니메이션 길이를 게이지 최대값으로 설정
	}

	EscapeGauge = 0.0f;
	GetWorld()->GetTimerManager().SetTimer(EscapeGaugeTimer, this, &AD1SurvivorBase::IncreaseEscapeGauge, 0.1f, true);
}

void AD1SurvivorBase::CancelEscapeAttempt()
{
	if (EscapeGauge >= MaxEscapeGauge)	return;

	if (!HasAuthority())
	{
		Server_CancelEscapeAttempt();
		return;
	}

	EscapeGauge = 0.0f;
	GetWorld()->GetTimerManager().ClearTimer(EscapeGaugeTimer);

	// 애니메이션 취소
	if (EscapeMontage)
	{
		PlayMontage(EscapeMontage, "CancelAttempt");
	}
}

void AD1SurvivorBase::IncreaseEscapeGauge()
{
	if (!HasAuthority()) return;

	EscapeGauge += 1.f; // 애니메이션 길이에 맞춰 0.1초씩 증가
	UE_LOG(LogTemp, Warning, TEXT("EscapeGauge: %f / %f"), EscapeGauge, MaxEscapeGauge);

	if (EscapeGauge >= MaxEscapeGauge)
	{
		GetWorld()->GetTimerManager().ClearTimer(EscapeGaugeTimer);
		AttemptEscape();
	}
}

void AD1SurvivorBase::Server_StartEscapeAttempt_Implementation()
{
	StartEscapeAttempt();
}

void AD1SurvivorBase::Server_CancelEscapeAttempt_Implementation()
{
	CancelEscapeAttempt();
}

void AD1SurvivorBase::AttemptEscape()
{
	if (!HasAuthority())
	{
		Server_AttemptEscape();
		return;
	}

	// 4% 확률로 탈출 성공
	float EscapeChance = FMath::RandRange(0.f, 100.f);
	//if (EscapeChance <= 99.f)
	if (EscapeChance <= 4.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Escape Success!"));
		PlayMontage(EscapeMontage, "Free");
		OnRescued();
		return;
	}

	// 탈출 실패 시 체력 16.66% 감소
	PlayMontage(EscapeMontage, "Fail");
	HookHealth -= (100.f / 6.f);
	EscapeGauge = 0.0f;
	UE_LOG(LogTemp, Warning, TEXT("Escape Failed! HookHealth: %f"), HookHealth);
}

void AD1SurvivorBase::Server_AttemptEscape_Implementation()
{
	AttemptEscape();
}

void AD1SurvivorBase::OnRescued()
{
	if (CurrentState == ESurvivorState::Hooked)
	{
		bIsCarryHook = false;
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Multicast_SkillCheckEnable(false);

		Multicast_StopEntityEvent();

		EscapeGauge = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(EscapeGaugeTimer);

		if (CurrentHook.IsValid())
		{
			CurrentHook->SetIsHooked(false);
			CurrentHook->SetInteractingPlayer(nullptr);
			CurrentHook = nullptr;
		}

		SetSurvivorState(ESurvivorState::Injured);
		GetCharacterMovement()->MaxWalkSpeed = GetSurvivoreSet()->GetInjRunSpeed();
	}
}

void AD1SurvivorBase::Die()
{
	if (CurrentState == ESurvivorState::Dying)
	{
		return; // 이미 사망한 경우 중복 실행 방지
	}

	// 생존자 상태 변경
	SetSurvivorState(ESurvivorState::Dying);

	// 사망 애니메이션 (TODO)

	// 5초 후 사망 처리 (게임에서 제거)
	GetWorld()->GetTimerManager().SetTimer(DeathRemoveTimer, this, &AD1SurvivorBase::RemoveFromGame, 10.0f, false);
}
void AD1SurvivorBase::DieFromBleedOut()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Log, TEXT("과다출혈로 사망!"));

	// TODO
}
void AD1SurvivorBase::DieFromEntity()
{
	if (CurrentState == ESurvivorState::Dying) return; // 이미 사망한 상태면 실행 X

	if (!HasAuthority())
	{
		Server_DieFromEntity();
		return;
	}

	Multicast_DieFromEntity();
}

void AD1SurvivorBase::Server_DieFromEntity_Implementation()
{
	Multicast_DieFromEntity();
}

void AD1SurvivorBase::Multicast_DieFromEntity_Implementation()
{
	DieFromEntity_Local();
}

void AD1SurvivorBase::DieFromEntity_Local()
{
	if (CurrentState == ESurvivorState::Dying) return; // 이미 사망한 상태면 실행 X

	if (HasAuthority())
	{
		Multicast_SkillCheckEnable(false);
	}
	UE_LOG(LogTemp, Log, TEXT("생존자 엔티티에 의해 사망!"));

	// 상태 변경
	SetSurvivorState(ESurvivorState::Dying);

	// 입력 비활성화
	if (AD1SurvivorController* PC = Cast<AD1SurvivorController>(GetController()))
	{
		DisableInput(PC);
		if (PC->IsLocalPlayerController())
		{
			PC->StopSurvivorBGM(1.0f);
		}
	}

	// 사망 애니메이션 재생
	if (SpiderMontage && CurrentHook.IsValid())
	{
		PlayAnimMontage(SpiderMontage, 1.0f, "Sacrifice");
		CurrentHook->PlayEntityMontage("Sacrifice");
	}
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AD1SurvivorBase::RemoveFromGame()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	AD1GameState* GS = GetWorld()->GetGameState<AD1GameState>();

	if (IsValid(PC) && IsValid(GS))
	{
		if (PC->IsLocalPlayerController())
		{
			GetController()->GetPawn()->SetActorHiddenInGame(true);
			GS->ResultSurvivorGame(PlayerIndex, ESurvivorState::Dying);
		}
	}
}

void AD1SurvivorBase::MoveToPalletStartPosition()
{
	if (!CurrentPallet.IsValid())	return;

	FVector PalletCenter = CurrentPallet->GetActorLocation();
	FVector PalletNormal = CurrentPallet->GetActorForwardVector();
	FVector SurvivorLocation = GetActorLocation();

	float OffsetDistance = -50.f; // 장애물 중앙 기준 앞으로 이동할 거리

	// 플레이어가 장애물 기준 앞쪽인지 뒤쪽인지 판단
	FVector ToPallet = (PalletCenter - SurvivorLocation).GetSafeNormal();
	float Dot = FVector::DotProduct(ToPallet, PalletNormal);

	if (Dot < 0)
	{
		PalletNormal *= -1; // 방향 반전
	}

	// 장애물 방향으로 이동
	FVector TargetLocation = PalletCenter - (PalletNormal * OffsetDistance);
	TargetLocation.Z += 88.f;  // 플레이어 위치 보정
	SetActorLocation(TargetLocation);

	// 장애물 방향으로 회전
	FRotator LookAtRotation = PalletNormal.Rotation();
	//LookAtRotation.Yaw += 180.f; // 장애물 좌표값 보정
	bUseControllerRotationYaw = true;
	SetActorRotation(LookAtRotation);
	bUseControllerRotationYaw = false;
}

void AD1SurvivorBase::MoveToExitGateStartPosition(AD1ExitGate* Gate)
{
	FVector TargetLocation = Gate->InteractionPoint->GetComponentLocation();
	TargetLocation.Z += 88.f;
	SetActorLocation(TargetLocation);

	FRotator LookAtRotation;
	LookAtRotation = (Gate->SwitchCollisionBox->GetComponentLocation() - TargetLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지

	bUseControllerRotationYaw = true;
	SetActorRotation(LookAtRotation);
	bUseControllerRotationYaw = false;
}


void AD1SurvivorBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this)
		return;

	if (!OverlappedActors.Contains(OtherActor))
	{
		OverlappedActors.Add(OtherActor);
	}

	UpdateClosestDetectedObject();

}

void AD1SurvivorBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OverlappedActors.Remove(OtherActor);

	if (DetectedObject == OtherActor)
	{
		DetectedObject = nullptr;

		if (AD1Generator* Generator = Cast<AD1Generator>(OtherActor))
		{
			Generator->StopRepair(this);
			CurrentGenerator = nullptr;
		}
		else if (AD1Pallet* Pallet = Cast<AD1Pallet>(OtherActor))
		{
			CurrentPallet = nullptr;
		}
		else if (OtherActor->ActorHasTag("Vaultable"))
		{
			VaultTarget = nullptr;
		}
	}

	UpdateClosestDetectedObject(); // 가장 가까운 새 대상 다시 선택
}

void AD1SurvivorBase::UpdateClosestDetectedObject()
{
	if (OverlappedActors.Num() == 0)
		return;

	// 후보 변수 선언
	AActor* NewDetected = nullptr;
	AD1Generator* NewGenerator = nullptr;
	AD1Pallet* NewPallet = nullptr;
	AD1VaultObject* NewVault = nullptr;

	AActor* ClosestGenerator = nullptr;
	AActor* ClosestPallet = nullptr;
	AActor* ClosestVault = nullptr;
	AActor* ClosestOther = nullptr;

	float DistSq_Generator = TNumericLimits<float>::Max();
	float DistSq_Pallet = TNumericLimits<float>::Max();
	float DistSq_Vault = TNumericLimits<float>::Max();
	float DistSq_Other = TNumericLimits<float>::Max();

	for (AActor* Actor : OverlappedActors)
	{
		if (!IsValid(Actor)) continue;

		float DistSq = FVector::DistSquared(Actor->GetActorLocation(), GetActorLocation());

		if (AD1Generator* Gen = Cast<AD1Generator>(Actor))
		{
			if (DistSq < DistSq_Generator)
			{
				DistSq_Generator = DistSq;
				ClosestGenerator = Gen;
			}
		}
		else if (AD1Pallet* Pallet = Cast<AD1Pallet>(Actor))
		{
			if (DistSq < DistSq_Pallet)
			{
				DistSq_Pallet = DistSq;
				ClosestPallet = Pallet;
			}
		}
		else if (Actor->ActorHasTag("Vaultable"))
		{
			if (DistSq < DistSq_Vault)
			{
				DistSq_Vault = DistSq;
				ClosestVault = Actor;
			}
		}
		else
		{
			if (DistSq < DistSq_Other)
			{
				DistSq_Other = DistSq;
				ClosestOther = Actor;
			}
		}
	}

	// 우선순위 적용
	if (ClosestGenerator)
	{
		NewDetected = ClosestGenerator;
		NewGenerator = Cast<AD1Generator>(ClosestGenerator);
	}
	else if (ClosestPallet)
	{
		NewDetected = ClosestPallet;
		NewPallet = Cast<AD1Pallet>(ClosestPallet);
	}
	else if (ClosestVault)
	{
		NewDetected = ClosestVault;
		NewVault = Cast<AD1VaultObject>(ClosestVault);
	}
	else if (ClosestOther)
	{
		NewDetected = ClosestOther;
	}

	// 실제 변경이 있는 경우에만 갱신
	if (NewDetected != DetectedObject)
	{
		DetectedObject = NewDetected;
		CurrentGenerator = NewGenerator;
		CurrentPallet = NewPallet;
		VaultTarget = NewVault;

		if (AD1ExitArea* ExitArea = Cast<AD1ExitArea>(DetectedObject))
		{
			if (ExitArea->IsActivated())
			{
				PlayEscapeSequence(ExitArea);
			}
		}
	}
}

void AD1SurvivorBase::Server_StartDropping_Request_Implementation(AD1Pallet* Pallet)
{
	Pallet->StartDropping(this);
}

void AD1SurvivorBase::TakeDamageFromKiller()
{
	switch (CurrentState)
	{
	case ESurvivorState::Healthy:
	{
		PlayMontage(HitMontage, "Hit_BK");
		SetSurvivorState(ESurvivorState::Injured);
		UE_LOG(LogTemp, Warning, TEXT("생존자가 부상 상태가 되었습니다!"));
		break;
	}

	case ESurvivorState::Injured:
	{
		// Crawl됐을 때 nullptr 해야할 변수들
		bIsRepairing = false;
		bPrevRepairing = false;
		bIsFail = false;
		bIsHealing = false;
		bIsUsingMedkit = false;
		bIsBeingHealed = false;

		PlayMontage(HitMontage, "BK");
		SetSurvivorState(ESurvivorState::Crawl);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		if (AD1SurvivorController* PC = Cast<AD1SurvivorController>(GetController()))
		{
			PC->PlaySurvivorBGMByLevel(EBGMLevel::Crawl);
		}
		UE_LOG(LogTemp, Warning, TEXT("생존자가 기절 상태가 되었습니다!"));
		break;
	}

	case ESurvivorState::Crawl:
	{
		SetSurvivorState(ESurvivorState::Crawl);
		UE_LOG(LogTemp, Warning, TEXT("생존자는 이미 기절 상태입니다!"));
		break;
	}

	}
}

void AD1SurvivorBase::TakePickUpFromKiller(AD1KillerBase* Killer)
{
	if (!Killer) return;

	SetSurvivorState(ESurvivorState::PickedUp);

	if (HasAuthority())
	{
		TakePickUpFromKiller_Multi(Killer);
	}
}

void AD1SurvivorBase::TakePickUpFromKiller_Multi_Implementation(AD1KillerBase* Killer)
{
	if (!Killer) return;

	// PickUp됐을 때 nullptr 해야할 변수들
	HealingSource = nullptr;
	bIsBeingHealed = false;
	bCanBeHealed = false;
	bIsCrawlSelfRecovering = false;

	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 물리 시뮬레이션 중지
	GetMesh()->SetSimulatePhysics(false);
	FName AttachSocketName = "PickUpSurvivor"; // 살인자의 왼손 본
	// 캐릭터를 본(소켓)에 부착
	AttachToComponent(Killer->GetCharacterMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);

}

void AD1SurvivorBase::TakeDropFromKiller(AD1KillerBase* Killer)
{
	if (!Killer) return;

	//TODO

	SetSurvivorState(ESurvivorState::Crawl);
}

void AD1SurvivorBase::BeingHealing(AD1SurvivorBase* Healer)
{
	if (!HasAuthority())
	{
		Server_BeingHealing(Healer);
		return;
	}

	BeingHealing_Local(Healer);
}

void AD1SurvivorBase::StopBeingHealing()
{
	if (!HasAuthority())
	{
		Server_StopBeingHealing();
		return;
	}

	StopBeingHealing_Local();
}

void AD1SurvivorBase::BeingHealing_Local(AD1SurvivorBase* Healer)
{
	if (!Healer) return;

	HealingSource = Healer;

	GetCharacterMovement()->DisableMovement();
	Healer->SetIsHealing(true);
	bIsBeingHealed = true;
}

void AD1SurvivorBase::StopBeingHealing_Local()
{
	HealingSource = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	bIsBeingHealed = false;
}

void AD1SurvivorBase::Server_BeingHealing_Implementation(AD1SurvivorBase* Healer)
{
	Multicast_BeingHealing(Healer);
}

void AD1SurvivorBase::Server_StopBeingHealing_Implementation()
{
	Multicast_StopBeingHealing();
}

void AD1SurvivorBase::Multicast_BeingHealing_Implementation(AD1SurvivorBase* Healer)
{
	if (!HasAuthority())
	{
		BeingHealing_Local(Healer);
	}
}
void AD1SurvivorBase::Multicast_StopBeingHealing_Implementation()
{
	if (!HasAuthority())
	{
		StopBeingHealing_Local();
	}
}

void AD1SurvivorBase::Server_SetSelfRecovering_Implementation(bool bNewState)
{
	bIsCrawlSelfRecovering = bNewState;
}

void AD1SurvivorBase::Client_PlayStartSequence_Implementation(float UNLOCK_INPUT_TIMER)
{
	FTimerHandle EndSequenceTimer;
	bPlayingIntro = true;
	OrbitSpeed = CurrentAngle / UNLOCK_INPUT_TIMER;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetRelativeRotation(FRotator(0.f, CurrentAngle, 0.f));

	GetWorld()->GetTimerManager().SetTimer(EndSequenceTimer, [this]() {
		bPlayingIntro = false;
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
		},
		UNLOCK_INPUT_TIMER,
		false);
}

void AD1SurvivorBase::FinishHealing()
{
	UE_LOG(LogTemp, Warning, TEXT("치료 완료!"));

	if (CurrentState == ESurvivorState::Injured)
		SetSurvivorState(ESurvivorState::Healthy);
	else if (CurrentState == ESurvivorState::Crawl)
		SetSurvivorState(ESurvivorState::Injured);

	if (HasAuthority())
	{
		if (HealingSource.IsValid())
		{
			Cast<AD1SurvivorController>(HealingSource->GetController())->StopHeal_Local(this);
		}
	}
	bCanBeHealed = false;
	GetWorldTimerManager().SetTimer(HealingCooldownTimer, this, &AD1SurvivorBase::ResetHealingCooldown, 0.5f, false);

	StopBeingHealing();
	// 치료 상태 초기화
	HealingProgress = 0.0f;
	bIsBeingHealed = false;
	HealingSource = nullptr;
}

// 아이템 장착 함수
void AD1SurvivorBase::EquipItem(TSubclassOf<AD1ItemBase> ItemClass)
{
	if (!ItemClass) return;

	// 기존 장착 아이템 제거
	if (EquippedItem)
	{
		EquippedItem->Destroy();
		EquippedItem = nullptr;
	}

	// 새 아이템 생성 및 장착
	EquippedItem = GetWorld()->SpawnActor<AD1ItemBase>(ItemClass);
	if (EquippedItem)
	{
		FName AttachSocketName = "RightHandItemSocket";  // 생존자의 오른손 소켓
		EquippedItem->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);

		EquippedItem->ItemOwner = this;
		EquippedItem->SetOwner(this);
		UE_LOG(LogTemp, Warning, TEXT("%s을(를) 장착했습니다."), *EquippedItem->GetName());
	}
}

// Server RPC
void AD1SurvivorBase::UseCurrentItem_Implementation()
{
	if (!EquippedItem) return;

	if (AD1Medkit* Medkit = Cast<AD1Medkit>(EquippedItem))
	{
		Medkit->UseItem(this);
	}
	else if (AD1Toolbox* Toolbox = Cast<AD1Toolbox>(EquippedItem))
	{
		Toolbox->UseItem(this);
	}
}

// Server
void AD1SurvivorBase::NotUseCurrentItem_Implementation()
{
	if (!EquippedItem) return;

	if (AD1Medkit* Medkit = Cast<AD1Medkit>(EquippedItem))
	{
		Medkit->NotUseItem(this);
	}
	else if (AD1Toolbox* Toolbox = Cast<AD1Toolbox>(EquippedItem))
	{
		Toolbox->NotUseItem(this);
	}
}

bool AD1SurvivorBase::ShouldShowItemMesh() const
{
	if (!(CurrentState == ESurvivorState::Healthy) &&
		!(CurrentState == ESurvivorState::Injured))
		return false;
	else if (CreatureState == ECreatureState::Parkour) // 파쿠르 할 때
		return false;
	else if (CreatureState == ECreatureState::DropPallet) // 판자 넘어뜨리기 할 때
		return false;
	else if (bIsRepairing) // 발전기 수리 중일 때
		return false;
	else if (bIsFail)	// 발전기 실패 모션 일 때
		return false;
	else if (bIsHealing) // 치료 중 일 때
		return false;
	else if (bIsBeingHealed) // 치료 받는 중 일 때
		return false;
	else if (bIsUsingMedkit) // 구급상자 사용 중 일 때
		return false;

	return true;
}

void AD1SurvivorBase::ResetHealingCooldown()
{
	bCanBeHealed = true;
	UE_LOG(LogTemp, Warning, TEXT("치료 가능 상태로 변경됨"));
}

void AD1SurvivorBase::MovePlayerToPalletPoint()
{
	AD1Pallet* Pallet = CurrentPallet.Get();
	EPalletLocation PalletLocation = Pallet->GetCurrentLocation();

	if (!HasAuthority())
	{
		Server_UpdatePalletLocation(Pallet, PalletLocation);
	}
	FVector TargetLocation;
	if (PalletLocation == EPalletLocation::LT)
	{
		TargetLocation = Pallet->InteractionPoint_Left->GetComponentLocation();
	}
	else
	{
		TargetLocation = Pallet->InteractionPoint_Right->GetComponentLocation();
	}

	// 플레이어 Z값 보정
	TargetLocation.Z += 88.f;

	SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

	FRotator LookAtRotation;

	LookAtRotation = (Pallet->InteractionPoint_Center->GetComponentLocation() - TargetLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지

	// 플레이어 회전
	bUseControllerRotationYaw = true;
	SetActorRotation(LookAtRotation);
	bUseControllerRotationYaw = false;
}

void AD1SurvivorBase::Server_UpdatePalletLocation_Implementation(AD1Pallet* Pallet, EPalletLocation PalletLocation)
{
	//Multicast_UpdatePalletLocation(Pallet, PalletLocation);
	Pallet->SetCurrentLocation(PalletLocation);
}

void AD1SurvivorBase::OnRep_SurvivorSet()
{
	if (SurvivorSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetWalkSpeed();
		GetCharacterMovement()->MaxWalkSpeedCrouched = SurvivorSet->GetCrouchSpeed();
	}
}

void AD1SurvivorBase::OnRep_EquippedItem()
{
	UE_LOG(LogTemp, Warning, TEXT("[클라이언트] EquippedItem이 복제되어 도착함: %s"), *GetNameSafe(EquippedItem));
}

void AD1SurvivorBase::Multicast_SkillCheckEnable_Implementation(bool State)
{
	bIsHookSkillCheckEnable = State;
	if (!GetController()) return;

	if (GetController()->IsLocalPlayerController())
	{
		BP_GetHook();
	}
}

void AD1SurvivorBase::PlayEscapeSequence(AD1ExitArea* ExitArea)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	AD1GameState* GS = GetWorld()->GetGameState<AD1GameState>();

	if (!IsValid(PC) || !IsValid(GS))
		return;

	if (PC->IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 탈출 시퀀스 시작"));

		// 콜리젼 없애기
		Server_PlayEscapeSequence(ExitArea);
		GetCapsuleComponent()->MoveIgnoreActors.Add(ExitArea);

		FRotator TargetRot = ExitArea->GetActorRotation();
		SetSurvivorState(ESurvivorState::Escape);

		ExitAreaFowardVector = ExitArea->GetActorForwardVector();
		CurrentAngle = 0.f;
		CurrentArmLength = SpringArm->TargetArmLength;

		OrbitSpeed = 60.f;
		ArmLengthSpeed = 60.f;

		SpringArm->bUsePawnControlRotation = false;
		SpringArm->bDoCollisionTest = false;

		bPlayingEscape = true;
		GS->ResultSurvivorGame(PlayerIndex, ESurvivorState::Escape);
	}
}
void AD1SurvivorBase::Server_PlayEscapeSequence_Implementation(AD1ExitArea* ExitArea)
{
	GetCapsuleComponent()->MoveIgnoreActors.Add(ExitArea);
}
void AD1SurvivorBase::SetSurvivorState(ESurvivorState state)
{
	PrevState = CurrentState;
	CurrentState = state;

	Server_SetSurvivorState(PlayerIndex, CurrentState);
}


void AD1SurvivorBase::Server_SetSurvivorState_Implementation(int32 _PlayerIndex, ESurvivorState State)
{
	AD1GameState* GS = GetWorld()->GetGameState<AD1GameState>();

	PrevState = CurrentState;
	CurrentState = State;

	if (IsValid(GS) && _PlayerIndex >= 0)
		GS->SetSurvivorState(_PlayerIndex, State);
}

AD1KillerBase* AD1SurvivorBase::GetKiller()
{
	if (!CachedKiller.IsValid())
	{
		for (TActorIterator<AD1KillerBase> It(GetWorld()); It; ++It)
		{
			CachedKiller = *It;
			break;
		}
	}

	return CachedKiller.Get();
}

void AD1SurvivorBase::TrySpawnScentSphere()
{
	if (GetVelocity().SizeSquared() < 10.0f) // 거의 안 움직이면 무시
		return;

	FVector TargetLocation = GetActorLocation();
	TargetLocation.Z -= 30.0f;

	GetWorld()->SpawnActor<class AD1ScentSphere>(
		AD1ScentSphere::StaticClass(),
		TargetLocation,
		FRotator::ZeroRotator
	);

	WolfCheck = true;
	//UE_LOG(LogTemp, Log, TEXT("🐾 향기 구체 생성됨: %s"), *GetName());
}