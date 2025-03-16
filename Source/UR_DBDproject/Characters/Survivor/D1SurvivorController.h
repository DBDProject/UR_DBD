// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1Define.h"
#include "D1SurvivorController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRepaireStart);

struct FInputActionValue;
class AD1SurvivorBase;
/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorController : public APlayerController
{
	GENERATED_BODY()
private:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Generator")
	FRepaireStart RepaireStartDelegate;

public:
	AD1SurvivorController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupInputComponent() override;

private:
	void Input_Move(const FInputActionValue& InputValue);
	void Input_Look(const FInputActionValue& InputValue);
	void Input_StartRun();
	void Input_StopRun();
	void Input_StartCrouch();
	void Input_StopCrouch();
	void Input_StartInteract_LeftClick();
	void Input_StopInteract_LeftClick();
	void Input_StartInteract_Space();
	void Input_StopInteract_Space();
	void Input_StartTestInput_1();

 
protected: // Run
	void StartRun_Local();
	void StopRun_Local();
	UFUNCTION(Server, Reliable)
	void Server_StartRun();
	UFUNCTION(Server, Reliable)
	void Server_StopRun();
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StartRun();
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StopRun();


protected: // Repair
	void StartRepair_Local();
	void StopRepair_Local();
	UFUNCTION(Server, Reliable)
	void Server_StartRepair();
	UFUNCTION(Server, Reliable)
	void Server_StopRepair();
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StartRepair();
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StopRepair();

public: // Heal
	void StartHeal_Local(AD1SurvivorBase* TargetSurvivor);
	void StopHeal_Local(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(Server, Reliable)
	void Server_StartHeal(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(Server, Reliable)
	void Server_StopHeal(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartHeal(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopHeal(AD1SurvivorBase* TargetSurvivor);

protected: // ExitGateOpen
	UFUNCTION()
	void StartExitOpening_Local();
	UFUNCTION()
	void StopExitOpening_Local();
	UFUNCTION(Server, Reliable)
	void Server_StartExitOpening(AD1ExitGate* Gate);
	UFUNCTION(Server, Reliable)
	void Server_StopExitOpening(AD1ExitGate* Gate);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartExitOpening(class AD1ExitGate* Gate);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopExitOpening(AD1ExitGate* Gate);

protected: // VaultObject
	UFUNCTION()
	void PerformVault(EVaultType VaultType);

protected: // Pallet
	void DropPallet();
	UFUNCTION()
	void VaultPallet();
public:
	UFUNCTION()
	void MoveToGeneratorPosition(EGeneratorInteractionPosition Position);


protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class AD1SurvivorBase> D1Survivor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UD1SurvivorBaseAnim> CachedAnimInstance;

private:
	bool bCanVaultAfterDrop = true; // DropPallet 후 VaultPallet을 막기 위한 변수
	FTimerHandle VaultCooldownTimer; // 타이머 핸들러 추가

	void EnableVaultAfterDrop(); // VaultPallet 활성화를 위한 함수


public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);

};
