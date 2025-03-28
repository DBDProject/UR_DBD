// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1ExitGate.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AD1ExitGate::AD1ExitGate()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 오버랩 감지 박스 (Interaction Box)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(100.f, 100.f, 150.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
	InteractionBox->SetGenerateOverlapEvents(true);

	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent);


	// 물리 충돌 박스
	DoorCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollisionBox"));
	DoorCollisionBox->SetupAttachment(DoorMesh);
	DoorCollisionBox->SetCollisionProfileName(TEXT("BlockAll"));  // 모든 물리 충돌 감지
	DoorCollisionBox->SetGenerateOverlapEvents(false); // 오버랩 판정 비활성화 (물리 전용)

	SwitchCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SwitchCollisionBox"));
	SwitchCollisionBox->SetupAttachment(DoorMesh);
	SwitchCollisionBox->SetCollisionProfileName(TEXT("BlockAll"));  // 모든 물리 충돌 감지
	SwitchCollisionBox->SetGenerateOverlapEvents(false); // 오버랩 판정 비활성화 (물리 전용)

	InteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint"));
	InteractionPoint->SetupAttachment(InteractionBox);

	CurrentState = EGateState::Closed;
}

// Called when the game starts or when spawned
void AD1ExitGate::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AD1ExitGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EGateState::SwitchActivation)
	{
		UpdateOpeningProgress(DeltaTime);
	}
}

void AD1ExitGate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1ExitGate, CurrentState);
	DOREPLIFETIME(AD1ExitGate, OpeningProgress);
	DOREPLIFETIME(AD1ExitGate, bActivateExitGate);
	DOREPLIFETIME(AD1ExitGate, bIsDoorOpened);

}

void AD1ExitGate::ActivateExitGate()
{
	if (bActivateExitGate == false)
	{
		bActivateExitGate = true;
	}
}


void AD1ExitGate::StartOpening(AD1SurvivorBase* Player)
{
	if (CurrentState == EGateState::Opened)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 탈출구가 열렸음!"));
		return;
	}

	if (CurrentState == EGateState::SwitchActivation)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 탈출구 레버 당기는 중!"));
		return;
	}

	if (HasAuthority())
	{
		StartOpening_Local(Player);
		return;
	}

	Server_StartExitOpening(Player);
}

void AD1ExitGate::StopOpening()
{
	if (HasAuthority())
	{
		StopOpening_Local();
		return;
	}

	Server_StopExitOpening();
}

void AD1ExitGate::StartOpening_Local(AD1SurvivorBase* Player)
{
	InteractingPlayer = Player;
	Player->SetIsExitGateOpening(true);
	CurrentState = EGateState::SwitchActivation;
}
void AD1ExitGate::StopOpening_Local()
{
	InteractingPlayer = nullptr;
	CurrentState = EGateState::Closed;
}
void AD1ExitGate::Server_StartExitOpening_Implementation(AD1SurvivorBase* Player)
{
	Multicast_StartExitOpening(Player);
}

void AD1ExitGate::Server_StopExitOpening_Implementation()
{
	Multicast_StopExitOpening();
}

void AD1ExitGate::Multicast_StartExitOpening_Implementation(AD1SurvivorBase* Player)
{
	if (!HasAuthority())
	{
		StartOpening_Local(Player);
	}
}

void AD1ExitGate::Multicast_StopExitOpening_Implementation()
{
	if (!HasAuthority())
	{
		StopOpening_Local();
	}
}

void AD1ExitGate::OpenDoor()
{
	if (CurrentState == EGateState::Opened)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 탈출구가 열렸음!"));
		return;
	}
	if (InteractingPlayer.IsValid())
	{
		InteractingPlayer->SetIsExitGateOpening(false);
		InteractingPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	InteractingPlayer = nullptr;
	CurrentState = EGateState::Opened;

	if (HasAuthority())
	{
		bIsDoorOpened = true;
		OnRep_DoorOpened();
	}

	UE_LOG(LogTemp, Warning, TEXT("탈출구 문이 열렸습니다!"));
}

void AD1ExitGate::OnRep_DoorOpened()
{
	if (bIsDoorOpened)
	{
		UE_LOG(LogTemp, Warning, TEXT("클라이언트에서 문 열림 처리됨"));
		DoorCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AD1ExitGate::UpdateOpeningProgress(float DeltaTime)
{
	if (!InteractingPlayer.IsValid())   return;

	OpeningProgress += DeltaTime / OpeningDuration;
	OpeningProgress = FMath::Clamp(OpeningProgress, 0.0f, 1.0f);

	UE_LOG(LogTemp, Warning, TEXT("탈출구 진행도: %.2f"), OpeningProgress);

	if (OpeningProgress >= 1.0f)
	{
		OpenDoor();
	}
}
