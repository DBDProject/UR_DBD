// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/D1PlayerSpawner.h"

// Sets default values
AD1PlayerSpawner::AD1PlayerSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 설정
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 메시 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Spawner"));
	MeshComponent->SetupAttachment(RootComponent); // 루트에 부착
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AD1PlayerSpawner::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 스포너 숨김
	MeshComponent->SetHiddenInGame(true);
}

// Called every frame
void AD1PlayerSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

