// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "D1CharacterBase.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1CharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AD1CharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:		// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> Camera;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 250.f;  // 기본 걷기 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 500.f;   // 달리기 속도

	UPROPERTY(BlueprintReadOnly)
	bool bIsRunning = false;

public:
	float GetWalkSpeed() const { return WalkSpeed; }
	void SetWalkSpeed(float NewSpeed) { WalkSpeed = NewSpeed; }

	float GetRunSpeed() const { return RunSpeed; }
	void SetRunSpeed(float NewSpeed) { RunSpeed = NewSpeed; }

	bool IsRunning() const { return bIsRunning; }
	void SetRunning(bool bRunning) { bIsRunning = bRunning; }
};
