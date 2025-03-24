// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/D1InGameMode.h"
#include "System/D1GameState.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Characters/Survivor/D1SurvivorController.h"
#include "Characters/Killer/D1KillerController.h"

AD1InGameMode::AD1InGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDelayedStart = true;
}

FCharacterDataSet* AD1InGameMode::GetCharacterData(ECharacterType CharacaterType)
{
	if (!m_dataTable)
		return nullptr;

	// Enum 값을 RowName으로 변환
	FName RowName = GetEnumRowName(CharacaterType);
	if (RowName.IsNone())
		return nullptr;

	static const FString ContextString(TEXT("PlayerCharacter Lookup"));
	return m_dataTable->FindRow<FCharacterDataSet>(RowName, ContextString);
}

APlayerController* AD1InGameMode::CreateControllerForCharacterType(UPlayer* NewPlayer, ECharacterType CharType)
{
	// 캐릭터 데이터 가져오기
	FCharacterDataSet* CharacterData = GetCharacterData(CharType);

	if (!CharacterData)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateControllerForCharacterType: 데이터를 찾을 수 없음!"));
		return nullptr;
	}

	APlayerController* NewController = GetWorld()->SpawnActor<APlayerController>(CharacterData->ControllerClass);

	if (!NewController)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateControllerForCharacterType: 컨트롤러 생성 실패!"));
		return nullptr;
	}


	NewController->SetPlayer(NewPlayer);
	NewController->SetReplicates(true);
	ConfigureController(NewController, CharacterData->PlayerStateClass, CharacterData->PawnClass);

	return NewController;
}

void AD1InGameMode::TeleportPlayerPawn(APlayerController* PlayerController)
{
	if (!PlayerController)
		return;

	if (Cast<AD1KillerController>(PlayerController))
	{
		SpawnKillerAtRandomSpawner(PlayerController);
	}
	else
	{
		SpawnSurvivorAtRandomSpawner(PlayerController);
	}
}

void AD1InGameMode::SpawnSurvivorAtRandomSpawner(APlayerController* PlayerController)
{
	if (!PlayerController) return;

	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SurvivorSpawner"), Spawners);

	if (Spawners.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("스포너가 더 이상 없음!"));
		return;
	}

	// 랜덤 선택
	int32 RandomIndex = FMath::RandRange(0, Spawners.Num() - 1);
	AActor* SelectedSpawner = Spawners[RandomIndex];

	// 플레이어 Pawn 가져오기
	APawn* SurvivorCharacter = Cast<APawn>(PlayerController->GetPawn());
	if (SurvivorCharacter && SelectedSpawner)
	{
		// 라인트레이스로 정확한 지면 찾기
		FVector Start = SelectedSpawner->GetActorLocation();
		FVector End = Start - FVector(0, 0, 10000);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;
		QueryParams.AddIgnoredActor(SurvivorCharacter);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECC_Visibility,
			QueryParams
		);

		if (bHit)
		{
			FVector GroundLocation = HitResult.Location + FVector(0, 0, 10.0f);
			SurvivorCharacter->SetActorLocation(GroundLocation);
			UE_LOG(LogTemp, Warning, TEXT("랜덤 스폰 지점에 배치: %s"), *SelectedSpawner->GetName());
		}
		else
		{
			SurvivorCharacter->SetActorLocation(SelectedSpawner->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("지면 감지 실패. 스포너 위치에 배치: %s"), *SelectedSpawner->GetName());
		}

		SelectedSpawner->Destroy();
	}
}

void AD1InGameMode::SpawnKillerAtRandomSpawner(APlayerController* PlayerController)
{
	if (!PlayerController) return;

	TArray<AActor*> Spawners;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("KillerSpawner"), Spawners);

	if (Spawners.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("스포너가 더 이상 없음!"));
		return;
	}

	// 랜덤 선택
	int32 RandomIndex = FMath::RandRange(0, Spawners.Num() - 1);
	AActor* SelectedSpawner = Spawners[RandomIndex];

	// 플레이어 Pawn 가져오기
	APawn* KillerCharacter = Cast<APawn>(PlayerController->GetPawn());
	if (KillerCharacter && SelectedSpawner)
	{
		// 라인트레이스로 정확한 지면 찾기
		FVector Start = SelectedSpawner->GetActorLocation();
		FVector End = Start - FVector(0, 0, 10000);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;
		QueryParams.AddIgnoredActor(KillerCharacter);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECC_Visibility,
			QueryParams
		);

		if (bHit)
		{
			FVector GroundLocation = HitResult.Location + FVector(0, 0, 10.0f);
			KillerCharacter->SetActorLocation(GroundLocation);
			UE_LOG(LogTemp, Warning, TEXT("랜덤 스폰 지점에 배치: %s"), *SelectedSpawner->GetName());
		}
		else
		{
			KillerCharacter->SetActorLocation(SelectedSpawner->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("지면 감지 실패. 스포너 위치에 배치: %s"), *SelectedSpawner->GetName());
		}

		SelectedSpawner->Destroy();
	}
}

void AD1InGameMode::ConfigureController(APlayerController* Controller,
	TSubclassOf<APlayerState> PSClass, TSubclassOf<APawn> PawnClass)
{
	if (!Controller || !PSClass || !PawnClass)
		return;

	APlayerState* NewPS = GetWorld()->SpawnActor<APlayerState>(PSClass);
	if (NewPS)
	{
		if (Controller->PlayerState)
			Controller->PlayerState->Destroy();

		Controller->PlayerState = NewPS;
		NewPS->SetOwner(Controller);
		NewPS->SetReplicates(true);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClass);

	if (NewPawn)
	{
		NewPawn->SetReplicates(true);
		NewPawn->SetReplicateMovement(true);

		if (Controller->GetPawn())
		{
			Controller->UnPossess();
			Controller->GetPawn()->Destroy();
		}

		Controller->Possess(NewPawn);
		UE_LOG(LogTemp, Warning, TEXT("컨트롤러 '%s'가 '%s'로 설정됨."), *Controller->GetName(), *NewPawn->GetName());
	}
}

void AD1InGameMode::ReadyPlayer()
{
	nReadyPlayerCount++;
	if (nReadyPlayerCount >= READY_PLAYER_COUNT)
	{
		StartMatch();
	}
}

FName AD1InGameMode::GetEnumRowName(ECharacterType CharacterType)
{
	const UEnum* EnumPtr = StaticEnum<ECharacterType>();
	if (!EnumPtr)
	{
		return NAME_None;
	}

	return FName(*EnumPtr->GetDisplayNameTextByValue((int64)CharacterType).ToString());
}

void AD1InGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	if (HasAuthority())
	{
		if (!m_dataTable)
		{
			// 캐릭터 데이터 테이블 로드
			m_dataTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/DBD/Data/CharacterDataTable.CharacterDataTable'"));
		}

		uint8 player = GetGameInstance<UD1GameInstance>()->m_serverInfo.maxPlayer;

		if (player > 1)
			READY_PLAYER_COUNT = GetGameInstance<UD1GameInstance>()->m_serverInfo.maxPlayer;

		UE_LOG(LogTemp, Warning, TEXT("현재 플레이어 매칭 수 : %d"), READY_PLAYER_COUNT);
	}
}

void AD1InGameMode::PreLogin(const FString& Options, const FString& Address,
	const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* AD1InGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	if (!NewPlayer)
	{
		ErrorMessage = TEXT("플레이어가 지정되지 않았습니다.");
		return nullptr;
	}

	// 캐릭터 타입 결정
	ECharacterType CharType;
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(NewPlayer);

	if (LocalPlayer)
		CharType = GetGameInstance<UD1GameInstance>()->m_serverInfo.killerInfo.characterType;
	else
		CharType = GetGameInstance<UD1GameInstance>()->m_serverInfo.survivorInfos[0].characterType;

	APlayerController* NewPlayerController = CreateControllerForCharacterType(NewPlayer, CharType);
	if (!NewPlayerController)
	{
		ErrorMessage = TEXT("컨트롤러 생성에 실패했습니다.");
		NewPlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	}

	return NewPlayerController;
}

void AD1InGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UD1GameInstance* gameInstance = GetGameInstance<UD1GameInstance>();

	if (IsValid(gameInstance))
	{
		// 아이피로 매칭해서 플레이어 인덱스를 게임 인스터스에 있는 서바이버 데이터와 동기화
		for (int32 i = 0; i < gameInstance->m_serverInfo.survivorInfos.Num(); i++)
		{
			if (gameInstance->m_serverInfo.survivorInfos[i].userIP ==
				NewPlayer->GetPlayerNetworkAddress())
			{
				AD1SurvivorBase* survivor = Cast<AD1SurvivorBase>(NewPlayer->GetPawn());

				if (IsValid(survivor))
					survivor->PlayerIndex = i;

				break;
			}
		}
	}

	ReadyPlayer();
}

void AD1InGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	nReadyPlayerCount--;

	if (nReadyPlayerCount <= 1)
	{
		EndMatch();
	}
}

void AD1InGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AD1InGameMode::RestartPlayer(AController* NewPlayer)
{
	// 플레이어 스타트 안쓰니 무시
}

void AD1InGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	TeleportPlayerPawn(NewPlayer);
}
