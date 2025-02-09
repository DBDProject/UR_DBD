// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1PlayerController.generated.h"

struct FInputActionValue;

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AD1PlayerController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	//	virtual void PlayerTick(float DeltaTime) override;

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);
	void Input_Run(const FInputActionValue& InputValue);

};
