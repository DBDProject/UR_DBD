// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Generator.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"

// Sets default values
AD1Generator::AD1Generator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 콜라이더 설정
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	RootComponent = InteractionBox;
	InteractionBox->SetBoxExtent(FVector(150.f, 150.f, 200.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));

	// 발전기 메쉬
	GeneratorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GeneratorMesh"));
	GeneratorMesh->SetupAttachment(RootComponent);

	// 기본값 설정
    InteractingPlayer = nullptr;
    bIsInteracting = false;
	bIsRepaired = false;
	RepairProgress = 0.f;
}

// Called when the game starts or when spawned
void AD1Generator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AD1Generator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // 발전기 수리 진행
    if (InteractingPlayer && RepairProgress < 100.f)
    {
        float RepairSpeed = InteractingPlayer->GetSurvivoreSet()->GetRepairSpeed();
        RepairProgress += RepairSpeed * DeltaTime;

        if (RepairProgress >= 100.f)
        {
            RepairProgress = 100.f;
            UE_LOG(LogTemp, Warning, TEXT("발전기 수리 완료!"));
        }
    }
}

EGeneratorInteractionPosition AD1Generator::FindInteractionPosition(AD1SurvivorBase* Survivor)
{
    if (!Survivor)
    {
        return EGeneratorInteractionPosition::Front; // 기본값
    }

    // 발전기 위치 및 방향 벡터 가져오기
    FVector GeneratorLocation = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector(); // 발전기의 "앞" 방향
    FVector RightVector = GetActorRightVector();     // 발전기의 "오른쪽" 방향

    // 플레이어 위치 가져오기
    FVector PlayerLocation = Survivor->GetActorLocation();

    // 플레이어 → 발전기로 향하는 방향 벡터 계산
    FVector DirectionToPlayer = (PlayerLocation - GeneratorLocation).GetSafeNormal();

    // 내적(Dot Product) 계산
    float ForwardDot = FVector::DotProduct(ForwardVector, DirectionToPlayer);
    float RightDot = FVector::DotProduct(RightVector, DirectionToPlayer);

    // 방향 판별
    if (ForwardDot > 0.7f)
    {
        return EGeneratorInteractionPosition::Front;  // 위(앞)
    }
    else if (ForwardDot < -0.7f)
    {
        return EGeneratorInteractionPosition::Back;  // 아래(뒤)
    }
    else if (RightDot > 0.7f)
    {
        return EGeneratorInteractionPosition::Right; // 오른쪽
    }
    else
    {
        return EGeneratorInteractionPosition::Left;  // 왼쪽
    }
}

void AD1Generator::StartRepair(AD1SurvivorBase* Player)
{
    if (!InteractingPlayer)
    {
        InteractingPlayer = Player;
        UE_LOG(LogTemp, Warning, TEXT("발전기 수리 시작!"));
    }
}

void AD1Generator::StopRepair()
{
    InteractingPlayer = nullptr;
    UE_LOG(LogTemp, Warning, TEXT("발전기 수리 중단!"));
}