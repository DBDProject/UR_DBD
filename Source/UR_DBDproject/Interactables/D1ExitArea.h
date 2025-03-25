// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"
#include "D1ExitArea.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1ExitArea : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(Replicated)
	bool bIsActivated = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> RootScene;

	// 물리 충돌 박스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ExitArea")
	TObjectPtr<class UBoxComponent> PhysicsCollisionBox;

	// 상호작용 범위 콜라이더
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ExitArea")
	TObjectPtr<class UBoxComponent> InteractionBox;

private:

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Sets default values for this actor's properties
	AD1ExitArea();

	void ActiavteExitArea();

	bool IsActivated() { return bIsActivated; }

};
