// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1CharacterBase.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"
#include "Interactables/D1Pallet.h"
#include "D1SurvivorBase.generated.h"

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChange, ESurvivorState,NewState);

UCLASS()
class UR_DBDPROJECT_API AD1SurvivorBase : public AD1CharacterBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnStateChange OnChangePlayerState;

public:
	AD1SurvivorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	virtual void InitAbilitySystem() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SmoothCameraTransition(float DeltaTime);
	void UpdateHealingProgress(float DeltaTime);
	void UpdateCrawlBleedOut(float DeltaTime);
	void UpdateHookBleedOut(float DeltaTime);

	void MoveToGeneratorPosition(EGeneratorInteractionPosition Position);
	void MoveToVaultStartPosition();
	void MoveToPalletStartPosition();
	void MoveToExitGateStartPosition(class AD1ExitGate* Gate);

	// 콜리전 이벤트 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public: // 발전기 수리
	UFUNCTION(BlueprintCallable)
	void StartRepair();
	UFUNCTION(BlueprintCallable)
	void StopRepair();
	UFUNCTION(Server, Reliable)
	void Server_RequestSkillCheckSuccess(class AD1Generator* Generator);
	UFUNCTION(Server, Reliable)
	void Server_RequestSkillCheckFail(class AD1Generator* Generator);
protected:
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
public:
	// 생존자 데미지 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void TakeDamageFromKiller();

	// 생존자 픽업 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void TakePickUpFromKiller(class AD1KillerBase* Killer);
	UFUNCTION()
	void TakePickUpFromKiller_Local(class AD1KillerBase* Killer);
	UFUNCTION(Server, Reliable)
	void TakePickUpFromKiller_Server(class AD1KillerBase* Killer);
	UFUNCTION(NetMulticast, Reliable)
	void TakePickUpFromKiller_Multicast(class AD1KillerBase* Killer);

	// 생존자 드랍 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void TakeDropFromKiller(class AD1KillerBase* Killer);

	// 생존자 치유 받는 함수
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void BeingHealing(AD1SurvivorBase* Healer);
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void StopBeingHealing();
	UFUNCTION()
	void BeingHealing_Local(AD1SurvivorBase* Healer);
	UFUNCTION()
	void StopBeingHealing_Local();
	UFUNCTION(Server, Reliable)
	void Server_BeingHealing(AD1SurvivorBase* Healer);
	UFUNCTION(Server, Reliable)
	void Server_StopBeingHealing();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BeingHealing(AD1SurvivorBase* Healer);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopBeingHealing();    
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateHealingProgress(float NewProgress);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateCrawlBleedOut(float NewProgress);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateHookBleedOut(float NewProgress);
	UFUNCTION(Server, Reliable)
	void Server_SetSelfRecovering(bool bNewState);

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void FinishHealing();

	// 아이템 장착 함수
	UFUNCTION(BlueprintCallable, Category = "Item")
	void EquipItem(TSubclassOf<AD1ItemBase> ItemClass);

	UFUNCTION(BlueprintCallable)
	void UseCurrentItem();

	void ResetHealingCooldown();

	void MovePlayerToPalletPoint();

	// 레플리케이션
	UFUNCTION()
	void OnRep_SurvivorSet();

	UFUNCTION()
	void OnRep_ChangeState();

	UFUNCTION(BlueprintImplementableEvent, Category = "State")
	void BP_OnHealthChanged();
public:  // 몽타주 실행
	UFUNCTION()
	void PlayMontage(UAnimMontage* Montage, FName SectionName);
protected: 
	void PlayMontage_Local(UAnimMontage* Montage, FName SectionName);
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* Montage, FName SectionName);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage, FName SectionName);

	UFUNCTION(Server, Reliable)
	void Server_UpdatePalletLocation(AD1Pallet* Pallet, EPalletLocation PalletLocation);
	
public: // 갈고리
	// 생존자 훅 처리 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartEntityEvent(class AD1SurvivorBase* Player);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopEntityEvent();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartEntityReaction();

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void StartOnHooked(class AD1Hook* Hook);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachToHook(class AD1Hook* Hook);
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void StartEscapeAttempt();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void CancelEscapeAttempt();

protected:
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void OnHooked();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void OnHookSkillCheckSuccess();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void OnHookSkillCheckFail();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void IncreaseEscapeGauge();
	UFUNCTION(Server, Reliable)
	void Server_StartEscapeAttempt();
	UFUNCTION(Server, Reliable)
	void Server_CancelEscapeAttempt();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void AttemptEscape();
	UFUNCTION(Server, Reliable)
	void Server_AttemptEscape();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void OnEscapeSuccess();

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void OnRescued();

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void Die();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void DieFromBleedOut();
	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void DieFromEntity();
	void DieFromEntity_Local();
	UFUNCTION(Server, Reliable)
	void Server_DieFromEntity();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DieFromEntity();

	UFUNCTION(BlueprintCallable, Category = "Survivor")
	void RemoveFromGame();


	// [[[[[[PROPERTY]]]]]]
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> VaultMontage; // 창 넘기기 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> PalletMontage; // 팔레트 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> HitMontage; // 히트 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> RescueMontage; // 구출 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> EscapeMontage; // 탈출 몽타주

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> SpiderMontage; // 엔티티 몽타주
protected:
	// 오버랩 감지용 박스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 생존자가 감지한 오브젝트 저장
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AActor> DetectedObject;

	// 상호작용 중인 발전기 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Generator> CurrentGenerator;

	// 상호작용 중인 창(VaultObject) 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1VaultObject> VaultTarget;

	// 상호작용 중인 팔레트 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Pallet> CurrentPallet;

	UPROPERTY(ReplicatedUsing = OnRep_SurvivorSet)
	TObjectPtr<UD1SurvivorSet> SurvivorSet;

	// 발전기 수리 중인지 여부
	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsRepairing = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bPrevRepairing = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	EGeneratorInteractionPosition InteractionPosition = EGeneratorInteractionPosition::None;

	// 스킬 체크 실패 시 
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bIsFail = false;

	// 생존자 상태 (건강, 부상, 기절)
	UPROPERTY(ReplicatedUsing = OnRep_ChangeState, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ESurvivorState CurrentState;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	ESurvivorState PrevState;

protected: // 치료 기능
	// 치료를 해주는 생존자
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AD1SurvivorBase> HealingSource = nullptr;

	// 치료 해주고 있는지
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bIsHealing = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	ESurvivorState HealingTargetState;

	// 현재 치료 진행도 (0~100%)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HealingProgress = 0.f;

	// 초당 치료 속도 (16초 동안 100% 완치)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HealingRate = 100.f / 16.f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bIsBeingHealed = false;

	// 치료가 가능한 상태인지 여부
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bCanBeHealed = true;

protected: // 빈사 상태
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor")
	float CrawlHealth = 100.0f; // 빈사 상태 HP

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor")
	bool bIsCrawlSelfRecovering = false; // 빈사 시 자가회복 여부

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor")
	float SelfRecoveryRate = 95.f / 30.f; // 자가회복 속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor")
	float BleedOutRate = 100.f / 240.f; // 출혈 속도

protected: // 갈고리
	// 걸려있는 갈고리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Hook> CurrentHook;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bIsCarryHook = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	int HookedCount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor")
	float HookBleedOutRate = 100.f / 120.f; // 갈고리 걸렸을 때 출혈 속도

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float HookHealth = 100.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bIsHookSkillCheckEnable = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	bool bIsHookSkillCheckFail = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Survivor", meta = (AllowPrivateAccess = "true"))
	float EscapeGauge = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Survivor")
	float MaxEscapeGauge = 100.0f;

	// 치료 불가 타이머 핸들
	FTimerHandle HealingCooldownTimer;
	// 사망 처리 타이머
	FTimerHandle DeathRemoveTimer;
	// 갈고리 탈출 게이지
	FTimerHandle EscapeGaugeTimer;
protected: // 탈출구
	// 탈출구 열고 있는지
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bIsExitGateOpening = false;

	// 현재 장착 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	TWeakObjectPtr<class AD1ItemBase> EquippedItem;

	// Temp (나중에 로비에서 선택하도록 바꿔야됨)
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AD1ItemBase> BP_MedkitClass;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class AD1ItemBase> BP_ToolboxClass;

public:
	AActor* GetDetectedObject() const { return DetectedObject.IsValid() ? DetectedObject.Get() : nullptr; }
	AD1Generator* GetCurrentGenerator() const { return CurrentGenerator.IsValid() ? CurrentGenerator.Get() : nullptr; }
	AD1VaultObject* GetVaultTarget() const { return VaultTarget.IsValid() ? VaultTarget.Get() : nullptr; }
	AD1Pallet* GetCurrentPallet() const { return CurrentPallet.IsValid() ? CurrentPallet.Get() : nullptr; }
	AD1Hook* GetCurrentHook() const { return CurrentHook.IsValid() ? CurrentHook.Get() : nullptr; }
	UD1SurvivorSet* GetSurvivoreSet() const { return SurvivorSet; }
	ESurvivorState GetSurvivorState() const { return CurrentState; }

	void SetSurvivorState(ESurvivorState state) { CurrentState = state; }

	void SetIsFail(bool state) { bIsFail = state; }
	void SetIsHealing(bool bNewState) { bIsHealing = bNewState; }
	void SetHealingTargetState(ESurvivorState State) { HealingTargetState = State; };
	bool GetCanBeHealed() { return bCanBeHealed; }

	bool GetIsSelfRecovering() { return bIsCrawlSelfRecovering; }
	void SetIsSelfRecovering(bool State) { bIsCrawlSelfRecovering = State; }
	void SetIsRepairing(bool state) { bIsRepairing = state; }
	void SetPrevRepairing(bool state) { bPrevRepairing = state; }
	float GetHookHealth() { return HookHealth; }

	EGeneratorInteractionPosition GetInteractionPosition() { return InteractionPosition; }
	void SetInteractionPosition(EGeneratorInteractionPosition NewPosition) { InteractionPosition = NewPosition; };

	UFUNCTION(BlueprintCallable)
	void SetIsExitGateOpening(bool bNewState) { bIsExitGateOpening = bNewState; }
};
