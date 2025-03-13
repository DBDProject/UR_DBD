// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/D1ServerController.h"
#include "Engine/LevelStreamingDynamic.h"
#include "System/D1GameState.h"

void AD1ServerController::BeginPlay()
{
	Super::BeginPlay();

	LoadAsyncGameMap();
}

void AD1ServerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LoadedLevel)
	{
		if (LoadedLevel->IsLevelLoaded())
		{
			UE_LOG(LogTemp, Warning, TEXT("레벨 로딩 완료!"));
			OnInGameMapLoaded();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("로컬 컨트롤러로 게임 맵 비동기 로딩 중..."));
		}
	}

}

void AD1ServerController::OnInGameMapLoaded()
{
	AD1GameState* D1GameState = GetWorld()->GetGameState<AD1GameState>();
	if (!D1GameState)
		return;

	//  맵이 화면에 표시되었는지 체크
	if (!LoadedLevel->IsLevelVisible())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnInGameMapLoaded: 맵이 아직 화면에 표시되지 않음."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("로컬 컨트롤러로 게임 맵 비동기 로딩 완료!"));

	GetWorld()->FlushLevelStreaming();
	GetWorld()->GetWorldSettings()->bForceNoPrecomputedLighting = false;

	LoadedLevel->SetShouldBeVisible(true);
	OnAsyncLoadMapEnd.Broadcast();
	D1GameState->ReadyPlayer();
	LoadedLevel = nullptr;
}

void AD1ServerController::LoadAsyncGameMap()
{
	if (!IsLocalController())
		return;

	AD1GameState* D1GameState = GetWorld()->GetGameState<AD1GameState>();

	if (!D1GameState)
		return;

	if (!D1GameState->bIsAsnycLoadMap)
		return;

	bool bOutSuccess = false;

	LoadedLevel = ULevelStreamingDynamic::LoadLevelInstance(
		GetWorld(),
		D1GameState->InGameMap,
		FVector(0.0f, 0.0f, 0.0f), // 원하는 위치
		FRotator::ZeroRotator,
		bOutSuccess,
		FString(),
		ULevelStreamingDynamic::StaticClass(),
		true
	);

	if (!LoadedLevel || !bOutSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("비동기 로딩 실패: '%s' 레벨을 로드할 수 없음!"), *D1GameState->InGameMap);
		return;
	}

	OnAsyncLoadMapStart.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("비동기 로딩 시작: '%s' 레벨을 로드 중..."), *D1GameState->InGameMap);
}

