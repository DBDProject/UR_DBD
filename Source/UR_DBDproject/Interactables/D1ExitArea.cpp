// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1ExitArea.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AD1ExitArea::AD1ExitArea()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// RootComponent 설정 (SceneComponent 사용)
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 물리 충돌 박스 (Physics Collision)
	PhysicsCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsCollisionBox"));
	PhysicsCollisionBox->SetupAttachment(RootComponent);
	PhysicsCollisionBox->SetBoxExtent(FVector(100.f, 800.f, 500.f)); // 적절한 크기로 설정
	PhysicsCollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	PhysicsCollisionBox->SetGenerateOverlapEvents(false); // 오버랩 판정 비활성화 (물리 전용)


	// 오버랩 감지 박스 (Interaction Box)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(250.f, 800.f, 500.f)); // 기존보다 살짝 크게
	InteractionBox->SetCollisionProfileName(TEXT("Interact")); // 인터랙트네
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화
}

void AD1ExitArea::ActiavteExitArea()
{
	bIsActivated = true;
}

void AD1ExitArea::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AD1ExitArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AD1ExitArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AD1ExitArea, bIsActivated);
}

