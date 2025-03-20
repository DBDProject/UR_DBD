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
}

void AD1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AD1GameState, RepairedGenerators);
	DOREPLIFETIME(AD1GameState, bAllGeneratorsRepaired);
	DOREPLIFETIME(AD1GameState, SurvivorInfos);
}

void AD1GameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AD1InGameMode* GameMode = Cast<AD1InGameMode>(AuthorityGameMode);
	// 게임 시작 시 발전기 수리해야할 개수 초기화
	if (HasAuthority())
	{
		RepairedGenerators = PlayerArray.Num();

		if (GetNetMode() == NM_ListenServer)
			OnRep_RepairedGenerators();

		FindPlayerSpawners();
		FindExitGates();
		SetPlayerLocation();

		UE_LOG(LogTemp, Warning, TEXT("==============게임 시작!============="));

		GetWorld()->GetTimerManager().SetTimer(InputLockTimer, this,
			&AD1GameState::OnInputUnlockTimer, GameMode->INPUT_UNLOCK_TIME, false);

	}

	Multi_SetInputLock(true);
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
			// 스포너 위치에서 지면까지 라인트레이스를 수행하여 정확한 지면 위치를 찾음
			FVector SpawnerLocation = SelectedKillerSpawner->GetActorLocation();
			FVector EndLocation = SpawnerLocation - FVector(0, 0, 10000); // 충분히 긴 거리로 아래 방향으로 트레이스

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.bTraceComplex = false;
			QueryParams.AddIgnoredActor(KillerCharacter);

			// 라인트레이스 수행
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				SpawnerLocation,
				EndLocation,
				ECC_Visibility,
				QueryParams
			);

			if (bHit)
			{
				// 지면 위치에 캐릭터를 배치하고, 약간의 오프셋을 추가하여 지면에 박히지 않도록 함
				FVector GroundLocation = HitResult.Location + FVector(0, 0, 10.0f);
				KillerCharacter->SetActorLocation(GroundLocation);
				UE_LOG(LogTemp, Warning, TEXT("서버 호스트를 킬러 스포너 위치의 지면에 배치: %s"), *SelectedKillerSpawner->GetName());
			}
			else
			{
				// 지면을 찾지 못한 경우 원래 스포너 위치를 사용
				KillerCharacter->SetActorLocation(SpawnerLocation);
				UE_LOG(LogTemp, Warning, TEXT("서버 호스트를 킬러 스포너 위치에 배치: %s (지면 감지 실패)"), *SelectedKillerSpawner->GetName());
			}
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
				// 스포너 위치에서 지면까지 라인트레이스를 수행하여 정확한 지면 위치를 찾음
				FVector SpawnerLocation = SelectedSurvivorSpawner->GetActorLocation();
				FVector EndLocation = SpawnerLocation - FVector(0, 0, 10000); // 충분히 긴 거리로 아래 방향으로 트레이스

				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.bTraceComplex = false;
				QueryParams.AddIgnoredActor(SurvivorCharacter);

				// 라인트레이스 수행
				bool bHit = GetWorld()->LineTraceSingleByChannel(
					HitResult,
					SpawnerLocation,
					EndLocation,
					ECC_Visibility,
					QueryParams
				);

				if (bHit)
				{
					// 지면 위치에 캐릭터를 배치하고, 약간의 오프셋을 추가하여 지면에 박히지 않도록 함
					FVector GroundLocation = HitResult.Location + FVector(0, 0, 10.0f);
					SurvivorCharacter->SetActorLocation(GroundLocation);
					UE_LOG(LogTemp, Warning, TEXT("플레이어 %d번을 생존자 스포너 위치의 지면에 배치: %s"), i, *SelectedSurvivorSpawner->GetName());
				}
				else
				{
					// 지면을 찾지 못한 경우 원래 스포너 위치를 사용
					SurvivorCharacter->SetActorLocation(SelectedSurvivorSpawner->GetActorLocation());
					UE_LOG(LogTemp, Warning, TEXT("플레이어 %d번을 생존자 스포너에 배치: %s (지면 감지 실패)"), i, *SelectedSurvivorSpawner->GetName());
				}
			}

			// 사용된 스포너 제거 (중복 방지)
			SurvivorSpawners.RemoveAt(RandomIndex);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("플레이어 위치 설정 완료!"));
}


void AD1GameState::OnInputUnlockTimer()
{
	if (!HasAuthority())
		return;

	Multi_SetInputLock(false);
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

void AD1GameState::OnRep_RepairedGenerators()
{
	UE_LOG(LogTemp, Warning, TEXT("발전기 수리 완료! 현재 수리된 발전기 개수: %d"), RepairedGenerators);
	OnGeneratorRepaired.Broadcast(RepairedGenerators);
}

void AD1GameState::OnRep_GeneratorCompleted()
{
	OnGeneratorCompleted.Broadcast();
}

//void AD1GameState::Multi_UpdateSurvivorStateUI_Implementation(const FSurvivorInfo& survivorInfo,
//	int32 playerIdx)
//{
//	UE_LOG(LogTemp, Warning, TEXT("생존자 상태 업데이트! : %d %d"), survivorInfo.survivorState, playerIdx);
//	OnSurvivorStateUpdated.Broadcast(survivorInfo, playerIdx);
//}

void AD1GameState::AddSurvivorInfo(APlayerController* Key, const FSurvivorInfo& SurvivorInfo)
{
	SurvivorIdxKey.Add(Key);
	SurvivorInfos.Add(SurvivorInfo);
}

int32 AD1GameState::GetSurvivorIndex(APlayerController* Key)
{
	for (int32 i = 0; i < SurvivorIdxKey.Num(); i++)
	{
		if (SurvivorIdxKey[i] == Key)
		{
			return i;
		}
	}

	return -1;
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

//void AD1GameState::Server_SetSurvivorState_Implementation(APlayerController* PlayerController, ESurvivorState NewState)
//{
//	if (!PlayerController)
//		return;
//
//	for (int32 i = 0; i < SurvivorIdxKey.Num(); i++)
//	{
//		if (SurvivorIdxKey[i] == PlayerController)
//		{
//			SurvivorInfos[i].survivorState = NewState;
//
//			if (NewState == ESurvivorState::Logout)
//				SurvivorIdxKey[i] = nullptr;
//
//			Multi_UpdateSurvivorStateUI(SurvivorInfos[i], i);
//			break;
//		}
//	}
//
//}

