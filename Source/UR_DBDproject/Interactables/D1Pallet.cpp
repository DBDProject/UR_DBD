// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Pallet.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/D1CharacterBase.h"
#include "Characters/Killer/D1KillerController.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AD1Pallet::AD1Pallet()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 오버랩 감지 박스 (Interaction Box)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(100.f, 100.f, 150.f)); // 기존보다 살짝 크게
	InteractionBox->SetCollisionProfileName(TEXT("Trigger")); // 오버랩 전용
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	// 오버랩 감지 박스 (Interaction Box)
	StunBox = CreateDefaultSubobject<UBoxComponent>(TEXT("StunBox"));
	StunBox->SetupAttachment(RootComponent);
	StunBox->SetBoxExtent(FVector(100.f, 100.f, 100.f)); // 기존보다 살짝 크게
	StunBox->SetCollisionProfileName(TEXT("Trigger")); // 오버랩 전용
	StunBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	PalletMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PalletMesh"));
	PalletMesh->SetupAttachment(RootComponent);

	InteractionPoint_Left = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint_Left"));
	InteractionPoint_Left->SetupAttachment(RootComponent);

	InteractionPoint_Right = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint_Right"));
	InteractionPoint_Right->SetupAttachment(RootComponent);

	InteractionPoint_Center = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint_Center"));
	InteractionPoint_Center->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PalletMontageAsset(TEXT("/Game/Blueprints/Animation/Interactables/AM_Pallet.AM_Pallet"));
	if (PalletMontageAsset.Succeeded())
	{
		PalletMontage = PalletMontageAsset.Object;
	}

	CurrentState = EPalletState::Up;
}

// Called when the game starts or when spawned
void AD1Pallet::BeginPlay()
{
	Super::BeginPlay();

	if (StunBox)
	{
		StunBox->OnComponentBeginOverlap.AddDynamic(this, &AD1Pallet::OnOverlapDropPalletBegin);
		StunBox->OnComponentEndOverlap.AddDynamic(this, &AD1Pallet::OnOverlapDropPalletEnd);
	}
}

void AD1Pallet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AD1Pallet, CurrentState);
}

// Called every frame
void AD1Pallet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

EPalletLocation AD1Pallet::FindClosestInteractionPoint(AD1CharacterBase* Player)
{
	if (!Player || !InteractionPoint_Left || !InteractionPoint_Right)
		return EPalletLocation::None;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector LeftPoint = InteractionPoint_Left->GetComponentLocation();
	FVector RightPoint = InteractionPoint_Right->GetComponentLocation();

	float DistToLeft = FVector::Dist(PlayerLocation, LeftPoint);
	float DistToRight = FVector::Dist(PlayerLocation, RightPoint);

	if (DistToLeft < DistToRight)
	{
		return EPalletLocation::LT;
	}
	else
	{
		return EPalletLocation::RT;
	}
}

EPalletLocation AD1Pallet::MovePlayerToInteractionPoint(AD1CharacterBase* Player, ECharacterType type)
{
	if (!Player) return EPalletLocation::None;

	EPalletLocation PalletLocation = FindClosestInteractionPoint(Player);
	if (EPalletLocation::None == PalletLocation) return EPalletLocation::None;

	FVector TargetLocation;
	if (PalletLocation == EPalletLocation::LT)
	{
		TargetLocation = InteractionPoint_Left->GetComponentLocation();
	}
	else
	{
		TargetLocation = InteractionPoint_Right->GetComponentLocation();
	}

	// 플레이어 Z값 보정

	FRotator LookAtRotation;

	if (type == ECharacterType::MEG)
	{
		TargetLocation.Z += 88.f;

		Player->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		LookAtRotation = (InteractionPoint_Center->GetComponentLocation() - TargetLocation).Rotation();
		LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
		LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
		// 플레이어 회전
		Player->SetActorRotation(LookAtRotation);
	}
	else if (type == ECharacterType::DRACULA)
	{
		TargetLocation.Z += 60.f;

		Player->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

		LookAtRotation = (InteractionPoint_Center->GetComponentLocation() - TargetLocation).Rotation();
		LookAtRotation.Pitch = -15.0f;
		if (!HasAuthority())
		{
			Server_SetControlRotation(Player, LookAtRotation);
		}
		else
		{
			AD1KillerController* KillerController = Cast<AD1KillerController>(Player->GetController());
			KillerController->SetControlRotation(LookAtRotation);

		}
	}


	return PalletLocation;
}

void AD1Pallet::PlayPalletMontage()
{
	if (!PalletMesh || !PalletMesh) return;
	PalletMesh->GetAnimInstance()->Montage_Play(PalletMontage);
}

void AD1Pallet::OnDestroy()
{
	Destroy();

	UE_LOG(LogTemp, Warning, TEXT("Pallet Destroyed"));
}

void AD1Pallet::StartDropping(AD1SurvivorBase* Player)
{
	if (!DeteactedKiller.IsValid()) return;

	if (!HasAuthority())
	{
		Player->Server_StartDropping_Request(this);
		return;
	}

	if (DeteactedKiller->GetCurrentTransformState() == EDraculaTransformationState::Dracula)
	{
		DeteactedKiller->ActivateAbility(D1GameplayTags::Killer_Ability_Dracula_Stun);
	}
	if (DeteactedKiller->GetCurrentTransformState() == EDraculaTransformationState::Wolf)
	{
		DeteactedKiller->ActivateAbility(D1GameplayTags::Killer_Ability_Wolf_Stun);
	}
	/*if (DeteactedKiller->GetCurrentTransformState() == EDraculaTransformationState::Bat)
	{
		DeteactedKiller->ActivateAbility(D1GameplayTags::Killer_Ability_Bat_Stun);
	}*/
}

void AD1Pallet::OnOverlapDropPalletBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (AD1KillerBase* Killer = Cast<AD1KillerBase>(OtherActor))
	{
		DeteactedKiller = Killer;
	}
}

void AD1Pallet::OnOverlapDropPalletEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AD1KillerBase* Killer = Cast<AD1KillerBase>(OtherActor))
	{
		DeteactedKiller = nullptr;
	}
}
void AD1Pallet::Server_SetControlRotation_Implementation(AD1CharacterBase* Player, FRotator LookAtRotation)
{
	if (!Player) return;

	AD1KillerController* KillerController = Cast<AD1KillerController>(Player->GetController());
	if (KillerController)
	{
		KillerController->SetControlRotation(LookAtRotation);
		UE_LOG(LogTemp, Log, TEXT("✅ SetControlRotation on Server: %s"), *LookAtRotation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ KillerController is NULL on Server!"));
	}
}
