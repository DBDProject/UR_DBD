// Fill out your copyright notice in the Description page of Project Settings.

/*
	Author : 변한빛
	Last Update : 2025-03-14
	Description : 소켓을 활용하는 레벨을 위한 게임모드 클래스
*/

#include "GameMode/D1NetGameMode.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void AD1NetGameMode::UpdateGameTime()
{
	if (!IsValid(m_pNetManager))
		return;

	if (m_pNetManager->IsConnected())
	{
		m_pNetManager->ProcessPacket();
	}
}

FString AD1NetGameMode::ConvertRelativeMapPath(const FString& RelativePath)
{
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FoundFiles;
	FString SearchPath = FPaths::ProjectContentDir(); // /Content/ 폴더 전체 검색

	// 재귀적으로 모든 폴더에서 .umap 파일 찾기
	FileManager.FindFilesRecursive(FoundFiles, *SearchPath, TEXT("*.umap"), true, false);

	for (const FString& FilePath : FoundFiles)
	{
		FString FoundMapName = FPaths::GetBaseFilename(FilePath);

		if (FoundMapName.Equals(RelativePath, ESearchCase::IgnoreCase))
		{
			FString RelativeContentPath = FilePath;
			FPaths::MakePathRelativeTo(RelativeContentPath, *FPaths::ProjectContentDir());

			// /Game/ 경로로 변환
			FString PackagePath = FString::Printf(TEXT("/Game/%s.%s"),
				*RelativeContentPath.LeftChop(5), // .umap 제거
				*FoundMapName);

			UE_LOG(LogTemp, Warning, TEXT("찾은 맵 경로: %s"), *PackagePath);
			return PackagePath;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("맵을 찾을 수 없음: %s"), *RelativePath);
	return FString();
}

void AD1NetGameMode::BeginPlay()
{
	Super::BeginPlay();
	m_pNetManager = GetGameInstance<UD1GameInstance>()->GetNetManager();
	GetWorld()->GetTimerManager().SetTimer(m_gameTimerHandle
		, this, &AD1NetGameMode::UpdateGameTime, 0.2f, true);
}

void AD1NetGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(m_gameTimerHandle);
	m_pNetManager = nullptr;
}

void AD1NetGameMode::OnLevelLoadComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("비동기 로딩 완료"));

	// 기존 맵 메모리 정리
	UGameplayStatics::UnloadStreamLevel(this, "PreviousMap", FLatentActionInfo(), false);
	GEngine->ForceGarbageCollection(true);
	bIsAsyncLoading = false;

	// 비동기 로딩 완료 이벤트 브로드캐스트
	OnAsyncLoadMapEnd.Broadcast();
}

void AD1NetGameMode::LoadAsyncGameMap(const FString& LevelName)
{
	if (bIsAsyncLoading)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 비동기 로딩 중입니다."));
		return;
	}

	m_asyncLoadMapName = ConvertRelativeMapPath(LevelName);
	FSoftObjectPath MapToLoad(m_asyncLoadMapName);

	if (!MapToLoad.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("에러: 유효하지 않은 맵 경로!  %s"), *LevelName);
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

	// 비동기 로딩 실행
	Streamable.RequestAsyncLoad(MapToLoad,
		FStreamableDelegate::CreateUObject(this, &AD1NetGameMode::OnLevelLoadComplete), 30);

	OnAsyncLoadMapStart.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("비동기 로딩 시작: '%s' 레벨을 로드 중..."), *LevelName);
	bIsAsyncLoading = true;
}
