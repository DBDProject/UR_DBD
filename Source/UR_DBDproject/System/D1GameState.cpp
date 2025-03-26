// Fill out your copyright notice in the Description page of Project Settings.


#include "System/D1GameState.h"
#include "Net/UnrealNetwork.h"
#include "Interactables/D1ExitGate.h"
#include "Interactables/D1ExitArea.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/D1PlayerSpawner.h"
#include "UI/D1GameStartUI.h"
#include "UI/D1GameEscapeUI.h"
#include "UI/D1GameExitUI.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Characters/Killer/D1KillerBase.h"

AD1GameState::AD1GameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f;
}
void AD1GameState::BeginPlay()
{
	Super::BeginPlay();

	bAllGeneratorsRepaired = false;
	IsGateOpened = false;
	ExitRemainingTime = GATE_EXIT_TIME;

	UD1GameInstance* GI = Cast<UD1GameInstance>(GetGameInstance());

	if (IsValid(GI))
		SurvivorStates.Init(ESurvivorState::Healthy, GI->m_serverInfo.maxPlayer);
}

void AD1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1GameState, RepairedGenerators);
	DOREPLIFETIME(AD1GameState, bAllGeneratorsRepaired);
	DOREPLIFETIME(AD1GameState, SurvivorStates);
	DOREPLIFETIME(AD1GameState, ExitRemainingTime);
}

void AD1GameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (HasAuthority())
	{
		RepairedGenerators = PlayerArray.Num();

		if (GetNetMode() == NM_ListenServer)
			OnRep_RepairedGenerators();

		GetWorld()->GetTimerManager().SetTimer(GameStartTimer, this,
			&AD1GameState::OnGameStartTimer, GAME_START_TIME, false);
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
		FindExitAreas();
		UE_LOG(LogTemp, Warning, TEXT("게임 대기 중!"));
	}

	if (IsValid(GameStartUIClass))
	{
		GameStartUI = CreateWidget<UD1GameStartUI>(GetWorld(), GameStartUIClass);
		GameStartUI->AddToViewport();
	}
}

void AD1GameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsGateOpened)
	{
		ExitRemainingTime -= DeltaSeconds;

		if (ExitRemainingTime <= 0.f)
		{
			OnGateEnded.Broadcast();
		}
	}
}

void AD1GameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	OnGameEnd.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("게임 종료!"));

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC)
		return;

	if (!PC->IsLocalController())
		return;

	AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(PC->GetPawn());

	if (IsValid(Survivor))
		ResultSurvivorGame(Survivor->PlayerIndex, ESurvivorState::Dying);
	else
		ResultKillerGame();
}

void AD1GameState::OnTravelTimer()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("L_Showcase3")));
}

void AD1GameState::OnPlayerTravelTimer()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("L_Showcase2")));
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

void AD1GameState::FindExitAreas()
{
	ExitAreas.Empty();

	TArray<AActor*> FoundAreas;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AD1ExitArea::StaticClass(), FoundAreas);

	for (AActor* ExitAreaActor : FoundAreas)
	{
		if (AD1ExitArea* ExitArea = Cast<AD1ExitArea>(ExitAreaActor))
		{
			ExitAreas.Add(ExitArea);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("맵에서 %d개의 탈출 지역을 찾음!"), ExitAreas.Num());
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

	if (!PC)
		return;

	AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(PC->GetPawn());
	AD1KillerBase* Killer = Cast<AD1KillerBase>(PC->GetPawn());

	if (IsValid(Survivor))
		Survivor->Client_PlayStartSequence(INPUT_UNLOCK_TIMER);
	else if (IsValid(Killer))
		Killer->PlayStartSequence(INPUT_UNLOCK_TIMER);


	UE_LOG(LogTemp, Warning, TEXT("게임 시작!"));

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

		for (AD1ExitArea* ExitArea : ExitAreas)
		{
			if (ExitArea)
			{
				ExitArea->ActiavteExitArea();
			}
		}
	}
}

void AD1GameState::SetSurvivorState(int32 PlayerIndex, ESurvivorState State)
{
	SurvivorStates[PlayerIndex] = State;
	Multi_SetSurvivorState(PlayerIndex, State);
}

void AD1GameState::ResultSurvivorGame(int32 PlayerIndex, ESurvivorState state)
{
	if (PlayerIndex < 0 && PlayerIndex >= SurvivorStates.Num())
		return;

	UD1GameInstance* GI = GetGameInstance<UD1GameInstance>();

	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	if (IsValid(Player))
		Player->DisableInput(Player);

	SurvivorStates[PlayerIndex] = state;

	if (IsValid(GI))
	{
		GI->m_resultInfo.isKiller = false;
		GI->m_resultInfo.playerIndex = PlayerIndex;
		GI->m_resultInfo.survivorStates = SurvivorStates;
		GI->m_resultInfo.survivorInfos = GI->m_serverInfo.survivorInfos;
	}

	GetWorld()->GetGameViewport()->RemoveAllViewportWidgets();

	if (state != ESurvivorState::Escape)
	{
		if (GameExitUIClass)
		{
			UD1GameExitUI* ExitUI = CreateWidget<UD1GameExitUI>(GetWorld(), GameExitUIClass);
			ExitUI->AddToViewport();
			ExitUI->GameExit();
		}

		GetWorld()->GetTimerManager().SetTimer(SurvivorTravelTimer, this,
			&AD1GameState::OnPlayerTravelTimer, NORMAL_EXIT_TIME, false);
	}
	else
	{
		if (GameEscapeUIClass)
		{
			UD1GameEscapeUI* EscapeUI = CreateWidget<UD1GameEscapeUI>(GetWorld(), GameEscapeUIClass);
			EscapeUI->AddToViewport();
			EscapeUI->GameEscape();
		}

		GetWorld()->GetTimerManager().SetTimer(SurvivorTravelTimer, this,
			&AD1GameState::OnPlayerTravelTimer, ESCAPE_EXIT_TIME, false);
	}
}

void AD1GameState::ResultKillerGame()
{
	UD1GameInstance* GI = GetGameInstance<UD1GameInstance>();

	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	if (IsValid(Player))
		Player->DisableInput(Player);

	if (GameExitUIClass)
	{
		UD1GameExitUI* ExitUI = CreateWidget<UD1GameExitUI>(GetWorld(), GameExitUIClass);
		ExitUI->AddToViewport();
		ExitUI->GameExit();
	}

	if (IsValid(GI))
	{
		GI->m_resultInfo.isKiller = true;
		GI->m_resultInfo.playerIndex = -1;
		GI->m_resultInfo.survivorStates = SurvivorStates;
		GI->m_resultInfo.survivorInfos = GI->m_serverInfo.survivorInfos;
	}

	// 서버장(리슨서버)은 잠깐 기다렸다가 나감
	GetWorld()->GetTimerManager().SetTimer(TravelTimer, this,
		&AD1GameState::OnTravelTimer, KILLER_EXIT_TIME, false);
}

void AD1GameState::Multi_SetSurvivorState_Implementation(int32 PlayerIndex, ESurvivorState State)
{
	UE_LOG(LogTemp, Warning, TEXT("생존자 상태 변경! : %d %d"), PlayerIndex, State);
	OnSurvivorStateChanged.Broadcast(PlayerIndex, State);
}

