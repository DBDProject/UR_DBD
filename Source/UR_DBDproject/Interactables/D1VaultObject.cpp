// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1VaultObject.h"
#include "Components/BoxComponent.h"

// Sets default values
AD1VaultObject::AD1VaultObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 장애물 메쉬
	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(RootComponent);
	ObstacleMesh->SetCollisionObjectType(ECC_GameTraceChannel2);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	HayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HayMesh"));
	HayMesh->SetupAttachment(RootComponent);
	HayMesh->SetCollisionObjectType(ECC_GameTraceChannel2);
	HayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	VaultTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("VaultTrigger"));
	VaultTrigger->SetupAttachment(RootComponent);
	VaultTrigger->SetCollisionProfileName(TEXT("Trigger"));
	VaultTrigger->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	Tags.Add("Vaultable"); // 창 감지용 태그
}

// Called when the game starts or when spawned
void AD1VaultObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AD1VaultObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

