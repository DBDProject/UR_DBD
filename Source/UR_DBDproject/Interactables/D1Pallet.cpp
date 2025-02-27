// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Pallet.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AD1Pallet::AD1Pallet()
{
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

    // 발전기 메쉬
    PalletMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PalletMesh"));
    PalletMesh->SetupAttachment(RootComponent);

    CurrentState = EPalletState::Up;
}

// Called when the game starts or when spawned
void AD1Pallet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AD1Pallet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

