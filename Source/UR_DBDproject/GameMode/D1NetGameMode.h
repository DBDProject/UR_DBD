/*
	Author : 변한빛
	Last Update : 2025-03-03
	Description : 소켓을 활용하는 레벨을 위한 게임모드 클래스
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "System/D1GameInstance.h"
#include "D1NetGameMode.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1NetGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
