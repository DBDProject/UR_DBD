// Fill out your copyright notice in the Description page of Project Settings.


#include "System/D1GameState.h"
#include "Net/UnrealNetwork.h"
#include "Interactables/D1ExitGate.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/D1PlayerSpawner.h"
#include "UI/D1GameStartUI.h"
#include "Characters/Survivor/D1SurvivorBase.h"

AD1GameState::AD1GameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}
void AD1GameState::BeginPlay()
{
	Super::BeginPlay();
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

	// 게임 시작 시 발전기 수리해야할 개수 초기화
	if (HasAuthority())
	{
		RepairedGenerators = PlayerArray.Num();

		if (GetNetMode() == NM_ListenServer)
			OnRep_RepairedGenerators();

		GetWorld()->GetTimerManager().SetTimer(GameStartTimer, this,
			&AD1GameState::OnGameStartTimer, 2.0f, false);
	}
}

void AD1GameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC->IsLocalController())
		PC->DisableInput(PC);

	if (HasAuthority())
	{
		FindExitGates();
		UE_LOG(LogTemp, Warning, TEXT("게임 대기 중!"));
	}

	if (IsValid(GameStartUIClass))
	{
		GameStartUI = CreateWidget<UD1GameStartUI>(GetWorld(), GameStartUIClass);
		if (GameStartUI)
			GameStartUI->AddToViewport();
	}
}

void AD1GameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	OnGameEnd.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("게임 종료!"));

	if (!HasAuthority())
	{
		// 로컬 클라이언트만 처리
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->IsLocalController())
		{
			PC->ClientTravel("L_Showcase2", ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		// 서버장(리슨서버)은 잠깐 기다렸다가 나감
		GetWorld()->GetTimerManager().SetTimer(TravelTimer, this, &AD1GameState::OnTravelTimer, 2.0f, false);
	}
}

void AD1GameState::OnTravelTimer()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->IsLocalController())
	{
		PC->ClientTravel("L_Showcase3", ETravelType::TRAVEL_Absolute);
	}
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

void AD1GameState::OnInputUnlockTimer()
{
	if (!HasAuthority())
		return;

	Multi_SetInputLock(false);
}

void AD1GameState::OnGameStartTimer()
{
	if (HasAuthority())
	{
		Multi_GameStart();

		GetWorld()->GetTimerManager().SetTimer(InputLockTimer, this,
			&AD1GameState::OnInputUnlockTimer, INPUT_UNLOCK_TIMER, false);
	}
}

void AD1GameState::Multi_SetInputLock_Implementation(bool bIsLock)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC->IsLocalController()) // 로컬 클라이언트만 적용
	{
		if (bIsLock)
		{
			PC->DisableInput(PC);
			UE_LOG(LogTemp, Warning, TEXT("클라이언트에서 입력 비활성화됨!"));
		}
		else
		{
			PC->EnableInput(PC);
			UE_LOG(LogTemp, Warning, TEXT("클라이언트에서 입력 활성화됨!"));
			OnInputUnlock.Broadcast();
		}
	}
}

void AD1GameState::Multi_GameStart_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());

	AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(Cast<APlayerController>(PC)->GetPawn());

	if (IsValid(Survivor))
		Survivor->Client_PlayStartSequence(INPUT_UNLOCK_TIMER);


	UE_LOG(LogTemp, Warning, TEXT("게임 시작!"));

	if (IsValid(GameStartUI))
		GameStartUI->GameStart();
	OnGameStart.Broadcast();
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

