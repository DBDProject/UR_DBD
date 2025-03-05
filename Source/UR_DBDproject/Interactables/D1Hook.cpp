// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Hook.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AD1Hook::AD1Hook()
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

	HookMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HookMesh"));
	HookMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AD1Hook::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AD1Hook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

