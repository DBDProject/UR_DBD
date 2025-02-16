// Fill out your copyright notice in the Description page of Project Settings.


#include "D1CharacterBase.h"

// Sets default values
AD1CharacterBase::AD1CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AD1CharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AD1CharacterBase::InitAbilitySystem()
{

}

// Called every frame
void AD1CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



