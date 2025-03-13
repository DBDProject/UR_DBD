// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1ExitGate.generated.h"

class AD1SurvivorBase;
UENUM(BlueprintType)
enum class EGateState : uint8
{
	Closed,				 // 닫힌 상태
	SwitchActivation,	 // 활성화 중
	Opened				 // 열린 상태
};

UCLASS()
class UR_DBDPROJECT_API AD1ExitGate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1ExitGate();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UFUNCTION(BlueprintCallable)
	void ActivateExitGate();

	// 탈출구 레버와 상호작용 시작
	UFUNCTION(BlueprintCallable)
	void StartOpening(class AD1SurvivorBase* InteractingPlayer);

	UFUNCTION(BlueprintCallable)
	void StopOpening();

	void StartOpening_Local(AD1SurvivorBase* Player);
	void StopOpening_Local();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateExitGate();
protected:
	UFUNCTION(Server, Reliable)
	void Server_StartExitOpening(AD1SurvivorBase* Player);
	UFUNCTION(Server, Reliable)
	void Server_StopExitOpening();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartExitOpening(AD1SurvivorBase* Player);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopExitOpening();

	UFUNCTION()
	void OnRep_DoorOpened();
private:
	void UpdateOpeningProgress(float DeltaTime);

	void OpenDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 상호작용 지점
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<class USceneComponent> InteractionPoint;

	// 스위치 물리 충돌 박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<class UBoxComponent> SwitchCollisionBox;
protected:
	// 상호작용 범위 (Collision)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 문 물리 충돌 박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<class UBoxComponent> DoorCollisionBox;

	// 문 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<class USkeletalMeshComponent> DoorMesh;

	// 문이 열렸는지 여부 (ReplicatedUsing)
	UPROPERTY(ReplicatedUsing = OnRep_DoorOpened)
	bool bIsDoorOpened;

	// 문 상태
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Door")
	EGateState CurrentState = EGateState::Closed;

	// 문이 열리는 진행도
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpeningProgress = 0.0f;

	// 문 열리는데 걸리는 시간 (기본 15초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpeningDuration = 15.0f;

	// 발전기 5개 완료 시 True (탈출구 활성화)
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bActivateExitGate = false;

	// 문을 여는 플레이어
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitGate")
	TWeakObjectPtr<class AD1SurvivorBase> InteractingPlayer;

public:
	EGateState GetCrrentState() { return CurrentState; }
	bool GetActivateExitGate() { return bActivateExitGate; }
};
