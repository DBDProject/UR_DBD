// Fill out your copyright notice in the Description page of Project Settings.


#include "System/D1GameState.h"
#include "Net/UnrealNetwork.h"
#include "Interactables/D1ExitGate.h"
#include "Kismet/GameplayStatics.h"

AD1GameState::AD1GameState()
{
	bReplicates = true;
}

void AD1GameState::BeginPlay()
{
	Super::BeginPlay();

	FindExitGates();
}

void AD1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1GameState, RepairedGenerators);
	DOREPLIFETIME(AD1GameState, bAllGeneratorsRepaired);
}

void AD1GameState::OnRep_RepairedGenerators()
{
	OnGeneratorRepaired.Broadcast(RepairedGenerators);
}

void AD1GameState::FindExitGates()
{
	ExitGates.Empty();

	TArray<AActor*> FoundGates;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AD1ExitGate::StaticClass(), FoundGates);

	for (AActor* GateActor : FoundGates)
	{
		if (AD1ExitGate* ExitGate = Cast<AD1ExitGate>(GateActor))
		{
			ExitGates.Add(ExitGate);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("맵에서 %d개의 탈출구를 찾음!"), ExitGates.Num());
}

void AD1GameState::UpdateGeneratorState_Implementation()
{
	if (!HasAuthority())
		return;

	++RepairedGenerators;
	UE_LOG(LogTemp, Warning, TEXT("현재 수리된 발전기 개수: %d"), RepairedGenerators);

	if (RepairedGenerators >= 5) // DBD는 발전기 5개 수리 필요
	{
		bAllGeneratorsRepaired = true;

		for (AD1ExitGate* ExitGate : ExitGates)
		{
			if (ExitGate)
			{
				ExitGate->ActivateExitGate();
			}
		}
	}
}
