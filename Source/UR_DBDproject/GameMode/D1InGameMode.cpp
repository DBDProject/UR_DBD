// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/D1InGameMode.h"
#include "System/D1GameState.h"

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

FName AD1InGameMode::GetEnumRowName(ECharacterType CharacterType)
{
	const UEnum* EnumPtr = StaticEnum<ECharacterType>();
	if (!EnumPtr)
	{
		return NAME_None;
	}

	return FName(*EnumPtr->GetDisplayNameTextByValue((int64)CharacterType).ToString());
}

void AD1InGameMode::GameStart()
{
	if (HasAuthority())
	{
		StartMatch();
	}
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

void AD1InGameMode::StartPlay()
{
	// 게임 스테이트에서 시작을 관리하기 위해 빈칸
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

