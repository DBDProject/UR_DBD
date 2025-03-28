// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"
#include "D1GA_Wolf_PowerAttack.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GA_Wolf_PowerAttack : public UD1GameplayAbility
{
	GENERATED_BODY()

public:
	UD1GA_Wolf_PowerAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> WolfCooldownEffect;

protected:
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> Wolf_PowerAttack;

private:
	void FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void EndDash();
	void DashToTarget();

	float ChargingStartTime = 0.0f;
	float WolfChargeDuration = 0.85f;

	FTimerHandle DashTimer;
	FTimerHandle DashEndTimer;

	void PerformWolfAttackTrace();
	bool bSurvivorHit = false;

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WolfAnim(AD1KillerBase* Player, FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WolfAnimStop(AD1KillerBase* Player, FName SectionName);

};
