// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1Define.h"
#include "D1Hook.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1Hook : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1Hook();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> RootScene;

	// 상호작용 범위 콜라이더
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
	TObjectPtr<class USkeletalMeshComponent> HookMesh;
};
