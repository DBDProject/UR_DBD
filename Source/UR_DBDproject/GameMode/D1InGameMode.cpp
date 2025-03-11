// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/D1InGameMode.h"

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


	AActor* StartSpot = FindRoleBasedPlayerStart();
	FVector SpawnLocation = StartSpot ? StartSpot->GetActorLocation() : FVector(0, 0, 100);
	FRotator SpawnRotation = StartSpot ? StartSpot->GetActorRotation() : FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
		PawnClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);

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
		UE_LOG(LogTemp, Warning, TEXT("플레이어 '%s'를 위치 %s에 스폰했습니다."),
			*Controller->GetName(), *SpawnLocation.ToString());
	}
}

AActor* AD1InGameMode::FindRoleBasedPlayerStart()
{
	// 찾을 태그 결정
	FName StartTag;

	//// 캐릭터 타입에 따라 적절한 태그 선택
	//if (CharType == ECharacterType::Killer)
	//{
	//	StartTag = FName("KillerStart");
	//}
	//else if (CharType >= ECharacterType::Survivor_Default && CharType <= ECharacterType::Survivor_4)
	//{
	//	// 생존자 수에 따라 다른 시작 위치 사용 가능
	//	int32 SurvivorIndex = static_cast<int32>(CharType) - static_cast<int32>(ECharacterType::Survivor_Default);
	//	StartTag = FName(*FString::Printf(TEXT("SurvivorStart_%d"), SurvivorIndex));
	//}
	//else
	//{
	//	StartTag = NAME_None; // 기본 PlayerStart 사용
	//}
	StartTag = NAME_None;

	// 맵에서 모든 PlayerStart 찾기
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	// 적절한 태그를 가진 PlayerStart 찾기
	for (AActor* Start : PlayerStarts)
	{
		// 태그가 지정되지 않았을 경우 기본 PlayerStart 검색
		if (StartTag == NAME_None)
		{
			return Start;
		}

		// 태그 확인
		APlayerStart* PlayerStart = Cast<APlayerStart>(Start);
		if (PlayerStart && PlayerStart->ActorHasTag(StartTag))
		{
			return PlayerStart;
		}
	}

	return nullptr;
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
	}
}

APlayerController* AD1InGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	if (!NewPlayer)
	{
		ErrorMessage = TEXT("플레이어가 지정되지 않았습니다.");
		return nullptr;
	}

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
		return nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 '%s'가 '%s'로 로그인했습니다."),
			*NewPlayer->GetName(), *NewPlayerController->GetName());
	}

	return NewPlayerController;
}

void AD1InGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AD1InGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

