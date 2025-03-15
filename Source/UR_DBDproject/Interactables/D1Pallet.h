// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1Define.h"
#include "D1Pallet.generated.h"

UENUM(BlueprintType)
enum class EPalletState : uint8
{
	Up,   // 세워진 상태
	Down, // 넘어져 있는 상태
	Destroyed, // 부서진 상태
};

UENUM(BlueprintType)
enum class EPalletLocation : uint8
{
	None,
	LT,   // 플레이어 기준 왼쪽에 팔레트
	RT, // 플레이어 기준 오른쪽에 팔레트
};

class USceneComponent;

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	EPalletLocation FindClosestInteractionPoint(class AD1CharacterBase* Player);
	EPalletLocation MovePlayerToInteractionPoint(class AD1CharacterBase* Player, ECharacterType type);

	UFUNCTION(BlueprintCallable)
	void OnDestroy();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RootScene;

	// 상호작용 범위 콜라이더
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pallet")
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pallet")
	TObjectPtr<class USkeletalMeshComponent> PalletMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	EPalletState CurrentState;

	// 팔레트가 왼쪽에 있을 때 상호작용 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	TObjectPtr<USceneComponent> InteractionPoint_Left;

	// 팔레트가 오른쪽에 있을 때 상호작용 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	TObjectPtr<USceneComponent> InteractionPoint_Right;

	// 중간 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	TObjectPtr<USceneComponent> InteractionPoint_Center;

public:
	EPalletState GetCurrentState() { return CurrentState; }
	void SetCurrentState(EPalletState State) { CurrentState = State; }

	UFUNCTION(Server, Reliable)
	void Server_SetControlRotation(class AD1CharacterBase* Player, FRotator LookAtRotation);
};
