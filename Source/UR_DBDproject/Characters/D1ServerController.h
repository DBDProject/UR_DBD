// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1ServerController.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncLoadMapStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncLoadMapEnd);

UCLASS()
class UR_DBDPROJECT_API AD1ServerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnAsyncLoadMapStart OnAsyncLoadMapStart;

	UPROPERTY(BlueprintAssignable, Category = "DBDListen")
	FOnAsyncLoadMapEnd OnAsyncLoadMapEnd;

protected:
	virtual void BeginPlay() override;

};
