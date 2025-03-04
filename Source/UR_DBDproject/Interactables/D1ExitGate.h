// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1ExitGate.generated.h"

UENUM(BlueprintType)
enum class EGateState : uint8
{
	Closed,       // 닫힌 상태
	Opening,      // 열리는 중
	Opened        // 열린 상태
};

UCLASS()
class UR_DBDPROJECT_API AD1ExitGate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1ExitGate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// 상호작용 범위 (Collision)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 문 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<class USkeletalMeshComponent> DoorMesh;

	// 상호작용 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<class USceneComponent> InteractionPoint;

	// 문 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	EGateState CurrentState;

	// 문이 열리는 진행도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	float OpeningProgress;
};
