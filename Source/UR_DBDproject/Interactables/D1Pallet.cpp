// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Pallet.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/D1CharacterBase.h"
#include "Characters/Killer/D1KillerController.h"
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

	PalletMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PalletMesh"));
	PalletMesh->SetupAttachment(RootComponent);

	InteractionPoint_Left = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint_Left"));
	InteractionPoint_Left->SetupAttachment(RootComponent);

	InteractionPoint_Right = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint_Right"));
	InteractionPoint_Right->SetupAttachment(RootComponent);

	InteractionPoint_Center = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint_Center"));
	InteractionPoint_Center->SetupAttachment(RootComponent);

	CurrentState = EPalletState::Up;
}

// Called when the game starts or when spawned
void AD1Pallet::BeginPlay()
{
	Super::BeginPlay();

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
	TargetLocation.Z += 88.f;

	Player->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

	FRotator LookAtRotation;

	if (type == ECharacterType::MEG)
	{
		LookAtRotation = (InteractionPoint_Center->GetComponentLocation() - TargetLocation).Rotation();
		LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
		LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
		// 플레이어 회전
		Player->SetActorRotation(LookAtRotation);
	}
	else if (type == ECharacterType::DRACULA)
	{
		LookAtRotation = (InteractionPoint_Center->GetComponentLocation() - TargetLocation).Rotation();
		LookAtRotation.Pitch = -15.0f;
		AD1KillerController* KillerController = Cast<AD1KillerController>(Player->GetController());
		KillerController->SetControlRotation(LookAtRotation);
	}


	return PalletLocation;
}

void AD1Pallet::OnDestroy()
{
	PalletMesh->SetHiddenInGame(true);
	UE_LOG(LogTemp, Warning, TEXT("Pallet Destroyed"));
}