// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1VaultObject.h"
#include "Components/BoxComponent.h"

// Sets default values
AD1VaultObject::AD1VaultObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VaultTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("VaultTrigger"));
	RootComponent = VaultTrigger;
	VaultTrigger->SetCollisionProfileName(TEXT("Trigger"));

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

