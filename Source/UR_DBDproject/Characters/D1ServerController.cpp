// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/D1ServerController.h"
#include "System/D1GameState.h"

void AD1ServerController::BeginPlay()
{
	Super::BeginPlay();

	OnAsyncLoadMapStart.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("Received : %s"), *GetName());
	OnAsyncLoadMapEnd.Broadcast();

	AD1GameState* GS = Cast<AD1GameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->ReadyPlayer();
	}
}
