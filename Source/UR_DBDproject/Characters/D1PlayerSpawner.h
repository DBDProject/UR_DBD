// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1PlayerSpawner.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1PlayerSpawner : public AActor
{
	GENERATED_BODY()

public:
	// 메시 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	UStaticMeshComponent* MeshComponent;

public:
	// Sets default values for this actor's properties
	AD1PlayerSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
