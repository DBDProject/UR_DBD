// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1ExitGate.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

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

    InteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint"));
    InteractionPoint->SetupAttachment(RootComponent);

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

}

