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
#include "Animation/D1SurvivorBaseAnim.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Interactables/D1Hook.h"
#include "Interactables/D1ExitGate.h"
#include "Items/D1ItemBase.h"
#include "Items/D1Medkit.h"
#include "Items/D1Toolbox.h"
#include "Net/UnrealNetwork.h"

AD1SurvivorBase::AD1SurvivorBase()
{
	UE_LOG(LogTemp, Warning, TEXT("생존자 생성됨! %s"), *GetName());
	auto a = this;
	GetCharacterMovement()->bOrientRotationToMovement = false;			// 이동 방향을 자동으로 바라보지 않음
	bUseControllerRotationYaw = false;									// 컨트롤러의 방향을 따라 캐릭터가 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 650.f, 0.f);	// 회전 속도


	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 200.f;
	SpringArm->bUsePawnControlRotation = true;				// 플레이어가 아니라 컨트롤러 회전을 따름

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false; // 카메라 독립적으로 회전 가능

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 상호작용 감지용 박스 컴포넌트 (상호작용 범위를 넓게 설정)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollider"));
	InteractionBox->SetupAttachment(GetCapsuleComponent());
	InteractionBox->SetBoxExtent(FVector(50.f, 50.f, 100.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

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
	static ConstructorHelpers::FObjectFinder<UAnimMontage> PickUpMontageAsset(TEXT("/Game/Blueprints/Animation/Survivor/AM_Meg_PickUp.AM_Meg_PickUp"));
	if (PickUpMontageAsset.Succeeded())
	{
		PickUpMontage = PickUpMontageAsset.Object;
	}

	CurrentState = ESurvivorState::Healthy;
}

void AD1SurvivorBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentState = ESurvivorState::Injured;
	// 컨트롤러의 기본 회전값을 설정하여 카메라 방향 조정
	if (Controller)
	{
		Controller->SetControlRotation(FRotator(-45, 0, 0));
	}

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
}

void AD1SurvivorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1SurvivorBase, SurvivorSet);
	DOREPLIFETIME(AD1SurvivorBase, bIsRepairing);
	DOREPLIFETIME(AD1SurvivorBase, InteractionPosition);
	DOREPLIFETIME(AD1SurvivorBase, bIsFail);
	DOREPLIFETIME(AD1SurvivorBase, bIsHealing);
	DOREPLIFETIME(AD1SurvivorBase, HealingTargetState);
	DOREPLIFETIME(AD1SurvivorBase, HealingProgress);
	DOREPLIFETIME(AD1SurvivorBase, bIsBeingHealed);
	DOREPLIFETIME(AD1SurvivorBase, bCanBeHealed);
	DOREPLIFETIME(AD1SurvivorBase, CurrentState);
	DOREPLIFETIME(AD1SurvivorBase, bIsExitGateOpening);
}

void AD1SurvivorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SmoothCameraTransition(DeltaTime);

	if (!HasAuthority()) return; // 서버에서만 실행

	if (bIsBeingHealed)
	{
		UpdateHealingProgress(DeltaTime);
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
	if (!bIsBeingHealed) return;

	// 치료 진행도 증가
	HealingProgress += HealingRate * DeltaTime;
	HealingProgress = FMath::Clamp(HealingProgress, 0.0f, 100.0f);

	Multicast_UpdateHealingProgress(HealingProgress);
	UE_LOG(LogTemp, Warning, TEXT("치료 진행도: %.2f%%"), HealingProgress);

	// 치료가 완료되었는지 확인
	if (HealingProgress >= 100.0f)
	{
		StopBeingHealing();
		FinishHealing();
	}
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
	SetActorRotation(LookAtRotation);
}

void AD1SurvivorBase::PlayMontage(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage) return;

	if (GetController()->IsLocalController())
	{
		PlayMontage_Local(Montage, SectionName);
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

	}
	else if (Montage == PickUpMontage)
	{

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
	SetActorRotation(LookAtRotation);
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

	SetActorRotation(LookAtRotation);
}

void AD1SurvivorBase::StartRunning()
{
	GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetRunSpeed();
}


void AD1SurvivorBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AD1Generator* Generator = Cast<AD1Generator>(OtherActor))
	{
		DetectedObject = OtherActor;
		CurrentGenerator = Generator;
	}

	if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("other survivor 감지"));
		DetectedObject = OtherActor;
	}

	if (AD1Pallet* Pallet = Cast<AD1Pallet>(OtherActor))
	{
		DetectedObject = OtherActor; 
		CurrentPallet = Pallet;
	}

	if (OtherActor->ActorHasTag("Vaultable"))
	{
		DetectedObject = OtherActor;
		VaultTarget = Cast<AD1VaultObject>(OtherActor);
	}

	if (AD1ExitGate* Gate = Cast<AD1ExitGate>(OtherActor))
	{

		UE_LOG(LogTemp, Warning, TEXT("DetectedGate"));
		DetectedObject = OtherActor;
	}
}

void AD1SurvivorBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (DetectedObject == OtherActor)
	{
		if (AD1Generator* Generator = Cast<AD1Generator>(DetectedObject.Get()))
		{
			Generator->StopRepair(this);
			CurrentGenerator = nullptr;
		}

		if (AD1Pallet* Pallet = Cast<AD1Pallet>(DetectedObject.Get()))
		{
			CurrentPallet = nullptr;
		}

		if (OtherActor->ActorHasTag("Vaultable"))
		{
			VaultTarget = nullptr;
		}

		DetectedObject = nullptr;
	}
}

void AD1SurvivorBase::TakeDamageFromKiller()
{
	switch (CurrentState)
	{
		case ESurvivorState::Healthy:
		{

			//GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetInjWalkSpeed();
			PlayAnimMontage(HitMontage, 1.0f, "Hit_BK");
			CurrentState = ESurvivorState::Injured;
			UE_LOG(LogTemp, Warning, TEXT("생존자가 부상 상태가 되었습니다!"));
			break;
		}


		case ESurvivorState::Injured:
		{
			//GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetCrawlSpeed();
			PlayAnimMontage(HitMontage, 1.0f);
			CurrentState = ESurvivorState::Crawl;
			UE_LOG(LogTemp, Warning, TEXT("생존자가 기절 상태가 되었습니다!"));
			break;
		}

		case ESurvivorState::Crawl:
		{
			UE_LOG(LogTemp, Warning, TEXT("생존자는 이미 기절 상태입니다!"));
			break;
		}

	}
}

void AD1SurvivorBase::TakePickUpFromKiller(AD1KillerBase* Killer)
{
	if (!Killer) return;

	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 물리 시뮬레이션 중지
	GetMesh()->SetSimulatePhysics(false);

	FName AttachSocketName = "joint_CarryLT_01"; // 살인자의 왼손 본

	// 캐릭터를 본(소켓)에 부착
	AttachToComponent(Killer->GetCharacterMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -1.07f), FRotator(0.f, 0.f, 0.f));

	SetSurvivorState(ESurvivorState::PickedUp);
}

void AD1SurvivorBase::OnHooked(AD1Hook* Hook)
{
	if (!Hook) return;
	if (CurrentState == ESurvivorState::Hooked) return;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	FName HookSocket = "socket_SurvivorHook";

	AttachToComponent(Hook->GetHookMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		HookSocket);

	GetMesh()->SetRelativeLocationAndRotation(FVector(65.f, 0.f, 0.f), FRotator(0.f, -90.f, 0.f));

	SetSurvivorState(ESurvivorState::Hooked);
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

void AD1SurvivorBase::Multicast_UpdateHealingProgress_Implementation(float NewProgress)
{
	HealingProgress = NewProgress;
}
void AD1SurvivorBase::FinishHealing()
{
	UE_LOG(LogTemp, Warning, TEXT("치료 완료!"));

	if (CurrentState == ESurvivorState::Injured)
		CurrentState = ESurvivorState::Healthy;
	else if (CurrentState == ESurvivorState::Crawl)
		CurrentState = ESurvivorState::Injured;

	bCanBeHealed = false;
	GetWorldTimerManager().SetTimer(HealingCooldownTimer, this, &AD1SurvivorBase::ResetHealingCooldown, 1.0f, false);

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
	if (EquippedItem.IsValid())
	{
		EquippedItem->Destroy();
		EquippedItem = nullptr;
	}

	// 새 아이템 생성 및 장착
	EquippedItem = GetWorld()->SpawnActor<AD1ItemBase>(ItemClass);
	if (EquippedItem.IsValid())
	{
		FName AttachSocketName = "RightHandItemSocket";  // 생존자의 오른손 소켓
		EquippedItem->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);

		UE_LOG(LogTemp, Warning, TEXT("%s을(를) 장착했습니다."), *EquippedItem->GetName());
	}
}

void AD1SurvivorBase::UseCurrentItem()
{
	if (EquippedItem.IsValid())
	{
		EquippedItem.Get()->UseItem(this);
	}
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
	SetActorRotation(LookAtRotation);
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


