// Fill out your copyright notice in the Description page of Project Settings.


#include "System/D1GameState.h"
#include "Net/UnrealNetwork.h"
#include "Interactables/D1ExitGate.h"
#include "Kismet/GameplayStatics.h"

AD1GameState::AD1GameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AD1GameState::BeginPlay()
{
	Super::BeginPlay();

	//RepairedGenerators = PlayerArray.Num();
	FindExitGates();
}

void AD1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1GameState, RepairedGenerators);
	DOREPLIFETIME(AD1GameState, bAllGeneratorsRepaired);
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

void AD1GameState::OnRep_RepairedGenerators()
{
	OnGeneratorRepaired.Broadcast(RepairedGenerators);
}

void AD1GameState::OnRep_GeneratorCompleted()
{
	OnGeneratorCompleted.Broadcast();
}

void AD1GameState::UpdateGeneratorState()
{
	if (!HasAuthority())
		return;

	if (RepairedGenerators > 0)
		RepairedGenerators--;

	// 리슨 서버의 경우 서버장이라 RepNotify가 호출되지 않음	
	if (GetNetMode() == NM_ListenServer)
		OnGeneratorRepaired.Broadcast(RepairedGenerators);

	UE_LOG(LogTemp, Warning, TEXT("현재 남은 발전기 개수: %d"), RepairedGenerators);

	if (RepairedGenerators <= 0) // DBD는 발전기 5개 수리 필요
	{
		bAllGeneratorsRepaired = true;

		if (GetNetMode() == NM_ListenServer)
			OnGeneratorCompleted.Broadcast();

		for (AD1ExitGate* ExitGate : ExitGates)
		{
			if (ExitGate)
			{
				ExitGate->Multicast_ActivateExitGate();
			}
		}
	}
}


