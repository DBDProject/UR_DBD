// Fill out your copyright notice in the Description page of Project Settings.


#include "System/D1GameState.h"
#include "Net/UnrealNetwork.h"
#include "Interactables/D1ExitGate.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/D1PlayerSpawner.h"

AD1GameState::AD1GameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AD1GameState::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("게임 상태 시작!"));
}

void AD1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1GameState, RepairedGenerators);
	DOREPLIFETIME(AD1GameState, bAllGeneratorsRepaired);
}

void AD1GameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (!HasAuthority())
		return;

	// 게임 시작 시 발전기 수리해야할 개수 초기화
	RepairedGenerators = PlayerArray.Num();

	if (GetNetMode() == NM_ListenServer)
		OnRep_RepairedGenerators();

	UE_LOG(LogTemp, Warning, TEXT("게임 시작! 발전기 수리해야할 개수: %d"), RepairedGenerators);
	FindPlayerSpawners();
	FindExitGates();
	SetPlayerLocation();

	// 게임 시작 델리게이트 호출
	OnGameStart.Broadcast();
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

void AD1GameState::FindPlayerSpawners()
{
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("KillerSpawner"), KillerSpawners);
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SurvivorSpawner"), SurvivorSpawners);

	// 찾은 액터 개수 출력
	UE_LOG(LogTemp, Warning, TEXT("킬러 스포너 개수: %d"), KillerSpawners.Num());
	UE_LOG(LogTemp, Warning, TEXT("생존자 스포너 개수: %d"), SurvivorSpawners.Num());
}

void AD1GameState::SetPlayerLocation()
{
	if (KillerSpawners.Num() == 0 || SurvivorSpawners.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("스포너를 찾을 수 없습니다!"));
		return;
	}

	// 서버 호스트를 랜덤한 킬러 스포너에 배치
	APlayerState* KillerPlayerState = PlayerArray.Num() > 0 ? PlayerArray[0] : nullptr;
	if (KillerPlayerState && KillerSpawners.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, KillerSpawners.Num() - 1);
		AActor* SelectedKillerSpawner = KillerSpawners[RandomIndex];

		APawn* KillerCharacter = Cast<APawn>(KillerPlayerState->GetPawn());
		if (KillerCharacter)
		{
			KillerCharacter->SetActorLocation(SelectedKillerSpawner->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("서버 호스트를 킬러 스포너 위치에 배치: %s"), *SelectedKillerSpawner->GetName());
		}
	}

	// 나머지 플레이어들을 랜덤한 생존자 스포너에 배치 (겹치지 않게)
	for (int32 i = 1; i < PlayerArray.Num(); i++) // 0번 플레이어는 킬러, 1번부터 생존자
	{
		APlayerState* SurvivorPlayerState = PlayerArray[i];
		if (SurvivorPlayerState && SurvivorSpawners.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, SurvivorSpawners.Num() - 1);
			AActor* SelectedSurvivorSpawner = SurvivorSpawners[RandomIndex];

			APawn* SurvivorCharacter = Cast<APawn>(SurvivorPlayerState->GetPawn());
			if (SurvivorCharacter)
			{
				SurvivorCharacter->SetActorLocation(SelectedSurvivorSpawner->GetActorLocation());
				UE_LOG(LogTemp, Warning, TEXT("플레이어 %d번을 생존자 스포너에 배치: %s"), i, *SelectedSurvivorSpawner->GetName());
			}

			// 사용된 스포너 제거 (중복 방지)
			SurvivorSpawners.RemoveAt(RandomIndex);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("플레이어 위치 설정 완료!"));
}

void AD1GameState::OnRep_RepairedGenerators()
{
	UE_LOG(LogTemp, Warning, TEXT("발전기 수리 완료! 현재 수리된 발전기 개수: %d"), RepairedGenerators);
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
		OnRep_RepairedGenerators();

	UE_LOG(LogTemp, Warning, TEXT("현재 남은 발전기 개수: %d"), RepairedGenerators);

	if (RepairedGenerators <= 0) // DBD는 발전기 5개 수리 필요
	{
		bAllGeneratorsRepaired = true;

		if (GetNetMode() == NM_ListenServer)
			OnRep_GeneratorCompleted();

		for (AD1ExitGate* ExitGate : ExitGates)
		{
			if (ExitGate)
			{
				ExitGate->Multicast_ActivateExitGate();
			}
		}
	}
}

