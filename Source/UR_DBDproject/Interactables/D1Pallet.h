// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1Pallet.generated.h"

UENUM(BlueprintType)
enum class EPalletState : uint8
{
	Up,   // 세워진 상태
	Down, // 넘어져 있는 상태
};

UCLASS()
class UR_DBDPROJECT_API AD1Pallet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1Pallet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FVector FindClosestInteractionPoint(class AD1CharacterBase* Player);
	void MovePlayerToInteractionPoint(class AD1CharacterBase* Player);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> RootScene;

	// 상호작용 범위 콜라이더
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pallet")
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pallet")
	TObjectPtr<class USkeletalMeshComponent> PalletMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pallet")
	EPalletState CurrentState;

	// 팔레트가 왼쪽에 있을 때 상호작용 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	USceneComponent* InteractionPoint_Left;

	// 팔레트가 오른쪽에 있을 때 상호작용 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	USceneComponent* InteractionPoint_Right;

	// 중간 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	USceneComponent* InteractionPoint_Center;

public:
	EPalletState GetCurrentState() { return CurrentState; }
	void SetCurrentState(EPalletState State) { CurrentState = State; }
};
