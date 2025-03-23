// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "D1GameStartUI.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API UD1GameStartUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void GameStart(); // 블루프린트에서 구현할 함수
};
