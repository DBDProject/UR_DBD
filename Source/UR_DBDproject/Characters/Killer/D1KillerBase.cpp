// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Killer/D1KillerController.h"
#include "Characters/Killer/D1KillerState.h"
#include "Animation/D1KillerBaseAnim.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Interactables/D1Generator.h"
#include "Interactables/D1VaultObject.h"
#include "Interactables/D1Pallet.h"
#include "Interactables/D1Hook.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "SkeletalMeshRestoreState.h"
#include "Sound/SoundAttenuation.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"
#include "Characters/Survivor/D1SurvivorController.h"
#include "D1KillerSoundManager.h"

AD1KillerBase::AD1KillerBase()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Dracula/SKM_Dracula.SKM_Dracula'")
	);
	if (CharacterMeshAsset.Succeeded())
	{
		CharacterMesh->SetSkeletalMesh(CharacterMeshAsset.Object);
		CharacterMesh->SetOwnerNoSee(true);
		CharacterMesh->SetupAttachment(GetCapsuleComponent());
		CharacterMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
		//CharacterMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load Character Mesh asset!"));
	}

	FPVMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPVMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FPVMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Dracula/SKM_Dracula_FPV.SKM_Dracula_FPV'")
	);
	if (FPVMeshAsset.Succeeded())
	{
		FPVMesh->SetSkeletalMesh(FPVMeshAsset.Object);
		FPVMesh->SetOnlyOwnerSee(true);
		FPVMesh->SetupAttachment(CharacterMesh);
		//FPVMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load FPVMesh asset!"));
	}

	BatMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BatMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BatMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Bat/SKM_Bat.SKM_Bat'")
	);
	if (BatMeshAsset.Succeeded())
	{
		BatMesh->SetSkeletalMesh(BatMeshAsset.Object);
		BatMesh->SetupAttachment(GetCapsuleComponent());
		BatMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 30.0f), FRotator(0.0f, -90.0f, 0.0f));
		BatMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
		BatMesh->SetHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load BatMesh asset!"));
	}

	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> WolfAsset(
			TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Wolf/SKM_Wolf.SKM_Wolf'")
		);
		GetMesh()->SetSkeletalMesh(WolfAsset.Object);
		GetMesh()->SetupAttachment(GetCapsuleComponent());
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
		GetMesh()->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
		GetMesh()->SetHiddenInGame(true);
	}
	
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 125.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bUsePawnControlRotation = true;				// 플레이어가 아니라 컨트롤러 회전을 따름
	SpringArm->bDoCollisionTest = false;

	BatSpringArm = CreateDefaultSubobject<USpringArmComponent>("BatSpringArm");
	BatSpringArm->SetupAttachment(BatMesh);
	BatSpringArm->TargetArmLength = 0.0f;
	BatSpringArm->bUsePawnControlRotation = true;				// 플레이어가 아니라 컨트롤러 회전을 따름
	BatSpringArm->bDoCollisionTest = false;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(SpringArm);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.f, 10.0f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	WolfCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("WolfCamera"));
	WolfCameraComponent->SetupAttachment(GetMesh());
	WolfCameraComponent->SetRelativeLocationAndRotation(FVector(0.f, -160.f, 140.0f), FRotator(0.0f, 90.0f, 0.0f)); // Position the camera
	WolfCameraComponent->bUsePawnControlRotation = true;
	WolfCameraComponent->Deactivate();

	BatCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("BatCamera"));
	BatCameraComponent->SetupAttachment(BatSpringArm);
	BatCameraComponent->SetRelativeLocationAndRotation(FVector(0.f, -150.f, 0.0f), FRotator(0.0f, 90.0f, 0.0f));  // Position the camera
	BatCameraComponent->bUsePawnControlRotation = true;
	BatCameraComponent->Deactivate();

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false; // 카메라 독립적으로 회전 가능
	Camera->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 90.f), FRotator(-30.0f, 0.0f, 0.0f)); // Position the camera

	// 상호작용 감지용 박스 컴포넌트 (상호작용 범위를 넓게 설정)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollider"));
	InteractionBox->SetupAttachment(GetCapsuleComponent());
	InteractionBox->SetBoxExtent(FVector(100.0f, 100.0f, 150.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	PowerAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PowerAttackCollision"));
	PowerAttackCollision->SetupAttachment(CharacterMesh); // 메쉬에 부착
	PowerAttackCollision->SetBoxExtent(FVector(0.3f, 0.3f, 0.3f));
	PowerAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PowerAttackCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PowerAttackCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	WolfPowerAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WolfPowerAttackCollision"));
	WolfPowerAttackCollision->SetupAttachment(GetMesh()); // 메쉬에 부착
	WolfPowerAttackCollision->SetBoxExtent(FVector(0.3f, 0.3f, 0.3f));
	WolfPowerAttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WolfPowerAttackCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	WolfPowerAttackCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint); 
	GetCharacterMovement()->SetWalkableFloorAngle(60.f); // 기본 44 -> 60으로 증가

	CharacterMesh->bEnableUpdateRateOptimizations = false;
	FPVMesh->bEnableUpdateRateOptimizations = false;
	GetMesh()->bEnableUpdateRateOptimizations = false;
	BatMesh->bEnableUpdateRateOptimizations = false;

	// 오디오 컴포넌트 생성
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComponent->SetupAttachment(RootComponent);

	// 안광
	{
		EyeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("EyeDecal"));
		EyeDecal->SetupAttachment(RootComponent);
		EyeDecal->DecalSize = FVector(500.f, 500.f, 500.f);
		EyeDecal->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalMat(TEXT("/Game/Materials/M_BloodDecal"));
		if (DecalMat.Succeeded())
		{
			EyeDecal->SetDecalMaterial(DecalMat.Object);
		}
		EyeDecal->SetVisibility(false);
	}
}

void AD1KillerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AD1KillerBase, bAttackDetectStart);
}

void AD1KillerBase::BeginPlay()
{
	Super::BeginPlay();

	// 카메라 활성화
	if (WolfCameraComponent) WolfCameraComponent->Deactivate();
	if (BatCameraComponent) BatCameraComponent->Deactivate();
	//if (FirstPersonCameraComponent) FirstPersonCameraComponent->Activate();
	if (FirstPersonCameraComponent) FirstPersonCameraComponent->Deactivate();
	if (Camera) Camera->Activate();
	//if (Camera) Camera->Deactivate();

	GetCharacterMesh()->bPauseAnims = false;
	GetFPVMesh()->bPauseAnims = false;
	GetMesh()->bPauseAnims = true;
	GetBatMesh()->bPauseAnims = true;

	GetCharacterMesh()->SetComponentTickEnabled(true);
	GetFPVMesh()->SetComponentTickEnabled(true);
	GetMesh()->SetComponentTickEnabled(false);
	GetBatMesh()->SetComponentTickEnabled(false);

	if (InteractionBox)
	{
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AD1KillerBase::OnOverlapObjectBegin);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AD1KillerBase::OnOverlapObjectEnd);
	}

	if (PowerAttackCollision)
	{
		PowerAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AD1KillerBase::OnPowerAttackOverlapPlayerBegin);
		PowerAttackCollision->OnComponentEndOverlap.AddDynamic(this, &AD1KillerBase::OnPowerAttackOverlapPlayerEnd);
	}

	if (WolfPowerAttackCollision)
	{
		WolfPowerAttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AD1KillerBase::OnPowerAttackOverlapPlayerBegin);
		WolfPowerAttackCollision->OnComponentEndOverlap.AddDynamic(this, &AD1KillerBase::OnPowerAttackOverlapPlayerEnd);
	}

	if (GetController())
	{
		if (GetController()->IsLocalController())
		{
			AudioComponent->SetVolumeMultiplier(0.0f);
			EyeDecal->SetVisibility(true);
		}
	}

	CurrentTransformState = EDraculaTransformationState::Dracula;

	// TEMP
	StartBGMUpdateTimer();
}

void AD1KillerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	{
		FVector Forward = GetActorForwardVector();
		FRotator DecalRotation = Forward.Rotation();
		EyeDecal->SetWorldRotation(DecalRotation);
	}

	if (!DetectedSurvivor.IsValid())
	{
	}
}

void AD1KillerBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystem();

	if (NewController->IsLocalController())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this; // 선택사항
		SpawnParams.Instigator = this;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FVector::ZeroVector);

		SoundManager = GetWorld()->SpawnActor<AD1KillerSoundManager>(
			SoundManagerClass,
			SpawnTransform,
			SpawnParams
		);
	}
}

void AD1KillerBase::InitAbilitySystem()
{
	Super::InitAbilitySystem();

	if (AD1KillerState* PS = GetPlayerState<AD1KillerState>())
	{
		AbilitySystemComponent = Cast<UD1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		AttributeSet = PS->GetD1KillerSet();
		KillerSet = Cast<UD1KillerSet>(AttributeSet);
		if (KillerSet)
		{
			GetCharacterMovement()->MaxWalkSpeed = KillerSet->GetWalkSpeed();
		}
	}
}

void AD1KillerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AD1KillerBase::HandleGameplayEvent(FGameplayTag EventTag)
{
	AD1KillerController* KC = Cast<AD1KillerController>(GetController());
	if (KC)
	{
		KC->HandleGameplayEvent(EventTag);
	}
}

void AD1KillerBase::SwitchCamera(EDraculaTransformationState NewState)
{
	if (Camera) Camera->Deactivate();
	if (FirstPersonCameraComponent) FirstPersonCameraComponent->Deactivate();
	if (WolfCameraComponent) WolfCameraComponent->Deactivate();
	if (BatCameraComponent) BatCameraComponent->Deactivate();

	if (NewState == EDraculaTransformationState::Dracula)
	{
		if (FirstPersonCameraComponent) FirstPersonCameraComponent->Activate();
		return;
	}
	if (NewState == EDraculaTransformationState::Wolf)
	{
		if (WolfCameraComponent) WolfCameraComponent->Activate();
		return;
	}
	if (NewState == EDraculaTransformationState::Bat)
	{
		if (BatCameraComponent) BatCameraComponent->Activate();
		return;
	}
}

void AD1KillerBase::OnOverlapObjectBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || DetectedObject == OtherActor) return;

	if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
	{
		if (Survivor->GetSurvivorState() == ESurvivorState::Crawl)
		{
			UE_LOG(LogTemp, Warning, TEXT("기절 상태의 생존자 감지"));
			DetectedObject = OtherActor;
			DetectedCrawlSurvivor = Survivor;
		}
	}
	else if (AD1Hook* Hook = Cast<AD1Hook>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("범위 내 훅 감지"));
		DetectedObject = OtherActor;
		CurrentHook = Hook;
	}
	else if (AD1Generator* Generator = Cast<AD1Generator>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Generator 감지"));
		DetectedObject = OtherActor;
		CurrentGenerator = Generator;
	}
	else if (AD1Pallet* Pallet = Cast<AD1Pallet>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Pallet 감지"));
		DetectedObject = OtherActor;
		CurrentPallet = Pallet;
	}
	else if (AD1VaultObject* VaultObj = Cast<AD1VaultObject>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("범위 내 VaultObject 감지"));
		DetectedObject = OtherActor;
		VaultTarget = VaultObj;
	}	
}

void AD1KillerBase::OnOverlapObjectEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || DetectedObject != OtherActor) return;

	if (CurrentGenerator == OtherActor)
	{
		CurrentGenerator = nullptr;
	}

	if (CurrentPallet == OtherActor)
	{
		CurrentPallet = nullptr;
	}

	if (CurrentHook == OtherActor)
	{
		CurrentHook = nullptr;
	}

	if (DetectedCrawlSurvivor == OtherActor)
	{
		DetectedCrawlSurvivor = nullptr;
	}

	if (VaultTarget == OtherActor)
	{
		VaultTarget = nullptr;
	}

	DetectedObject = nullptr;
}

void AD1KillerBase::OnPowerAttackOverlapPlayerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!PowerAttackCollision->IsActive())
		return;

	if (bAttackSuccess)
		return;

	if (OtherActor && OtherActor != this && DetectedObject != OtherActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Power Attack 적중: %s"), *OtherActor->GetName());
		if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("Power Attack Survivor 감지됨: %s"), *Survivor->GetName());
			Survivor->TakeDamageFromKiller();
			bAttackSuccess = true;
		}
	}
}

void AD1KillerBase::OnPowerAttackOverlapPlayerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AD1KillerBase::OnWolfPowerAttackOverlapPlayerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!WolfPowerAttackCollision->IsActive())
		return;

	if (bAttackSuccess)
		return;

	if (OtherActor && OtherActor != this && DetectedObject != OtherActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Wolf Power Attack 적중: %s"), *OtherActor->GetName());
		if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("Wolf Power Attack Survivor 감지됨: %s"), *Survivor->GetName());
			Survivor->TakeDamageFromKiller();
			bAttackSuccess = true;
		}
	}
}

void AD1KillerBase::OnWolfPowerAttackOverlapPlayerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AD1KillerBase::DamageSurvivor(class AD1SurvivorBase* Player)
{
	if (!DetectedSurvivor.IsValid()) return;

	

	DetectedSurvivor->TakeDamageFromKiller();

}

void AD1KillerBase::ActivateAbility(FGameplayTag AbilityTag)
{
	AbilitySystemComponent->ActivateAbility(AbilityTag);
}

void AD1KillerBase::OnRep_KillerSet()
{
	if (KillerSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = KillerSet->GetWalkSpeed();
	}
}

void AD1KillerBase::PerformDraculaAttackTrace()
{
	bSurvivorHit = false;
	FVector TraceStart = GetMesh()->GetSocketLocation("joint_FingerBRT_01");
	FVector TraceEnd = TraceStart + (GetActorForwardVector() * 150.f);

	float TraceRadius = 75.f;

	// 트레이스 결과값 저장용
	TArray<FHitResult> HitResults;
	ECollisionChannel TraceChannel = ECC_Pawn;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);


	// SphereTraceMulti
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	// For Debugging
	//float DebugDrawTime = 2.f;
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, DebugDrawTime);
	//DrawDebugSphere(GetWorld(), TraceEnd, TraceRadius, 12, FColor::Red, false, DebugDrawTime);

	if (bHit)
	{
		for (auto& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != this)
			{
				UE_LOG(LogTemp, Warning, TEXT("공격 적중! : %s"), *HitActor->GetName());
				if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(HitActor))
				{
					UE_LOG(LogTemp, Warning, TEXT("서바이버에게 피해 적용: %s"), *Survivor->GetName());

					Survivor->TakeDamageFromKiller();
					bSurvivorHit = true;

					break;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("공격 미스!"));
	}
}

void AD1KillerBase::PerformWolfAttackTrace()
{
	bSurvivorHit = false;
	FVector TraceStart = GetMesh()->GetSocketLocation("nose");
	FVector TraceEnd = TraceStart + (GetActorForwardVector() * 150.f);

	float TraceRadius = 50.0f;

	// 트레이스 결과값 저장용
	TArray<FHitResult> HitResults;
	ECollisionChannel TraceChannel = ECC_Pawn;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);


	// SphereTraceMulti
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	// For Debugging
	//float DebugDrawTime = 2.f;
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, DebugDrawTime);
	//DrawDebugSphere(GetWorld(), TraceEnd, TraceRadius, 12, FColor::Red, false, DebugDrawTime);

	if (bHit)
	{
		for (auto& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != this)
			{
				UE_LOG(LogTemp, Warning, TEXT("공격 적중! : %s"), *HitActor->GetName());
				if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(HitActor))
				{
					UE_LOG(LogTemp, Warning, TEXT("서바이버에게 피해 적용: %s"), *Survivor->GetName());

					Survivor->TakeDamageFromKiller();
					bSurvivorHit = true;

					break;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("공격 미스!"));
	}
}

void AD1KillerBase::UpdateSurvivorBGMStates()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState) return;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		AD1SurvivorController* SurvivorPC = Cast<AD1SurvivorController>(PS->GetOwner());
		if (!SurvivorPC || SurvivorPC->IsLocalController()) continue;

		APawn* SurvivorPawn = SurvivorPC->GetPawn();
		if (!SurvivorPawn) continue;

		float DistSq = FVector::DistSquared(GetActorLocation(), SurvivorPawn->GetActorLocation());

		EBGMLevel DetectedLevel;

		if (DistSq <= FMath::Square(800.0f))         // 8m
			DetectedLevel = EBGMLevel::Terror;
		else if (DistSq <= FMath::Square(2000.0f))    // 16m
			DetectedLevel = EBGMLevel::Threat;
		else if (DistSq <= FMath::Square(4000.0f))   // 32m
			DetectedLevel = EBGMLevel::Warning;
		else
			DetectedLevel = EBGMLevel::Normal;

		FBGMStateInfo& Info = SurvivorBGMMap.FindOrAdd(SurvivorPC);

		// 상태가 바뀌었으면 대기 상태로 전환
		if (Info.PendingState != DetectedLevel)
		{
			Info.PendingState = DetectedLevel;
			Info.StateDuration = 0.f;
		}
		else
		{
			Info.StateDuration += 0.5f;

			// 일정 시간 유지되면 전환
			constexpr float ThresholdTime = 1.0f;
			if (Info.PendingState != Info.LastConfirmedState && Info.StateDuration >= ThresholdTime)
			{
				// 킬러 BGM
				if (Info.PendingState == EBGMLevel::Terror)
				{
					if (CurrentBGMState != EBGMLevel::Terror)
					{
						SoundManager->PlayBGM(ChaseBGM, 1.0f);
						CurrentBGMState = Info.PendingState;
					}
				}
				else
				{
					if (CurrentBGMState != EBGMLevel::Normal)
					{
						SoundManager->PlayBGM(NormalBGM, 2.0f);
						CurrentBGMState = EBGMLevel::Normal;
					}
				}
				SurvivorPC->Client_UpdateBGMLevel(Info.PendingState);
				Info.LastConfirmedState = Info.PendingState;
			}
		}
	}
}

void AD1KillerBase::StartBGMUpdateTimer()
{
	if (HasAuthority()) // 서버에서만 실행
	{
		GetWorldTimerManager().SetTimer(
			SurvivorBGMUpdateTimerHandle,
			this,
			&AD1KillerBase::UpdateSurvivorBGMStates,
			0.5f,     // 주기
			true      // 루프
		);
	}
}
