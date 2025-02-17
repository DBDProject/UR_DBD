// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1Define.h"
#include "D1SurvivorController.generated.h"

struct FInputActionValue;
/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorController : public APlayerController
{
	GENERATED_BODY()

public:
	AD1SurvivorController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);
	void Input_RunStart();
	void Input_RunStop();
	void Input_StartCrouch();
	void Input_StopCrouch();

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AD1SurvivorBase> D1Survivor;

public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);
};
