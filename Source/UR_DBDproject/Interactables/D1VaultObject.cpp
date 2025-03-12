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

void AD1VaultObject::MoveToVaultInteractionLocation(AD1CharacterBase* Character)
{
	if (!Character || !VaultTrigger) return;
	FVector VaultLocation = VaultTrigger->GetComponentLocation();
	FVector RightVector = VaultTrigger->GetRightVector();
	FVector ForwardVector = VaultTrigger->GetForwardVector();

	//상호작용 위치 계산
	FVector CharacterLocation = Character->GetActorLocation();

	StartPos = VaultLocation + (CharacterLocation - VaultLocation).GetSafeNormal() * 100.f;

	// 플레이어가 장애물의 앞쪽에 있는지 판단
	FVector ToObstacle = (VaultLocation - CharacterLocation).GetSafeNormal();
	float RightdDot = FVector::DotProduct(RightVector, ToObstacle);

	if (RightdDot > 0)
	{
		StartPos = VaultLocation - RightVector * 100.0f;
		TargetPos = VaultLocation + RightVector * 100.0f;
	}
	else
	{
		StartPos = VaultLocation + RightVector * 100.0f;
		TargetPos = VaultLocation - RightVector * 100.0f;
	}

	Character->SetActorLocation(StartPos);
}

