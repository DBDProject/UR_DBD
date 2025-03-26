// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "D1Define.h"
#include "D1SurvivorController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRepaireStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRepaireEnd);

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
	UPROPERTY(BlueprintAssignable, Category = "Generator")
	FRepaireStart RepaireStartDelegate;


	UPROPERTY(BlueprintAssignable, Category = "Generator")
	FRepaireEnd RepaireEndDelegate;

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
	void Input_StartInteract_RightClick();
	void Input_StopInteract_RightClick();
	void Input_PointTo();
	void Input_ComeHere();

public: // Repair
	void RepairDelegate_Start();
	void RepairDelegate_End();
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

protected: // Rescue
	void StartRescue_Local(AD1SurvivorBase* TargetSurvivor);
	void StopRescue_Local(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(Server, Reliable)
	void Server_StartRescue(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(Server, Reliable)
	void Server_StopRescue(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartRescue(AD1SurvivorBase* TargetSurvivor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopRescue(AD1SurvivorBase* TargetSurvivor);

public:
	UFUNCTION()
	void MoveToGeneratorPosition(EGeneratorInteractionPosition Position);


protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class AD1SurvivorBase> D1Survivor;

protected: // 사운드
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AD1SurvivorSoundManager> SoundManagerClass;

	// 로컬 사운드 매니저
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AD1SurvivorSoundManager> SoundManager;

	// [[[[사운드 트랙]]]]
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBGMLevel CurrentBGMLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> NormalBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> WarningBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> ThreatBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> TerrorBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> CrawlBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> HookBGM_Part1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> HookBGM_Part2;
private:
	bool bCanVaultAfterDrop = true; // DropPallet 후 VaultPallet을 막기 위한 변수
	FTimerHandle VaultCooldownTimer; // 타이머 핸들러 추가

	void EnableVaultAfterDrop(); // VaultPallet 활성화를 위한 함수


public:
	ECreatureState GetCreatureState();
	void SetCreatureState(ECreatureState InState);
	
// 사운드
	UFUNCTION(Client, Reliable)
	void Client_UpdateBGMLevel(EBGMLevel NewLevel);


	UFUNCTION(Client, Reliable)
	void Client_PlaySurvivorBGMByLevel(EBGMLevel NewLevel);

	UFUNCTION(BlueprintCallable)
	void PlaySurvivorBGMByLevel(EBGMLevel NewLevel);
};
