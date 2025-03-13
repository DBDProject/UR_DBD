// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/D1GameInstance.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "D1InGameMode.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1InGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBDListen", Meta = (Displayername = "CharacterTable"))
	TObjectPtr<UDataTable> m_dataTable;

private:
	APlayerController* CreateControllerForCharacterType(UPlayer* NewPlayer, ECharacterType CharType);

	void ConfigureController(APlayerController* Controller, TSubclassOf<APlayerState> PSClass,
		TSubclassOf<APawn> PawnClass);

public:
	AD1InGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FCharacterDataSet* GetCharacterData(ECharacterType CharacaterType);
	FName GetEnumRowName(ECharacterType CharacterType);

	void GameStart();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
		const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void Logout(AController* Exiting) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

