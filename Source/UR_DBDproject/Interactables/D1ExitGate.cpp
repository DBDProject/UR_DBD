// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1ExitGate.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"

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

void AD1ExitGate::ActivateExitGate()
{
    if (bActivateExitGate == false)
    {
        bActivateExitGate = true;
        UE_LOG(LogTemp, Warning, TEXT("탈출구 레버가 활성화됨!"));
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

    InteractingPlayer = Player;
    if (!InteractingPlayer.IsValid()) return;

    UE_LOG(LogTemp, Warning, TEXT("탈출구 문 열기 시작"));
    CurrentState = EGateState::SwitchActivation;
}

void AD1ExitGate::StopOpening()
{
    UE_LOG(LogTemp, Warning, TEXT("탈출구 문 열기 중지"));
    InteractingPlayer = nullptr;
    CurrentState = EGateState::Closed;
}

void AD1ExitGate::MovePlayerToInteractionPoint(AD1CharacterBase* Player)
{
    if (!Player) return;

    FVector TargetLocation = InteractionPoint->GetComponentLocation();
    TargetLocation.Z += 88.f;
    Player->SetActorLocation(TargetLocation);

    FRotator LookAtRotation;
    LookAtRotation = (SwitchCollisionBox->GetComponentLocation() - TargetLocation).Rotation();
    LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
    LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
    Player->SetActorRotation(LookAtRotation);
}

void AD1ExitGate::OpenDoor()
{
    if (CurrentState == EGateState::Opened)
    {
        UE_LOG(LogTemp, Warning, TEXT("이미 탈출구가 열렸음!"));
        return;
    }

    InteractingPlayer = nullptr;
    CurrentState = EGateState::Opened;
    DoorCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UE_LOG(LogTemp, Warning, TEXT("탈출구 문이 열렸습니다!"));
}

void AD1ExitGate::UpdateOpeningProgress(float DeltaTime)
{
    if (!InteractingPlayer.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("상호 작용중인 플레이어 없음"));
        return;
    }

    OpeningProgress += DeltaTime / OpeningDuration;
    OpeningProgress = FMath::Clamp(OpeningProgress, 0.0f, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("탈출구 진행도: %.2f"), OpeningProgress);

    if (OpeningProgress >= 1.0f)
    {
        OpenDoor();
    }
}


