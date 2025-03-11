// Fill out your copyright notice in the Description page of Project Settings.


#include "D1CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

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

	AddCharacterAbilities();
}

void AD1CharacterBase::InitAbilitySystem()
{

}

// Called every frame
void AD1CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AD1CharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void AD1CharacterBase::HandleGameplayEvent(FGameplayTag EventTag)
{
}

void AD1CharacterBase::AddCharacterAbilities()
{
	UD1AbilitySystemComponent* ASC = Cast<UD1AbilitySystemComponent>(AbilitySystemComponent);
	if (ASC == nullptr)
	{
		return;
	}

	ASC->AddCharacterAbilities(StartupAbilities);
}
