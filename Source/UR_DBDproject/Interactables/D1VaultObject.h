// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1VaultObject.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1VaultObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1VaultObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> RootScene;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UBoxComponent> VaultTrigger;

	// 장애물 매쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UStaticMeshComponent> ObstacleMesh;

	// 지푸라기 매쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UStaticMeshComponent> HayMesh;
};
