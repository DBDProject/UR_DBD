// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"
#include "D1GA_Wolf_SecondPowerAttack.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GA_Wolf_SecondPowerAttack : public UD1GameplayAbility
{
	GENERATED_BODY()

public:
	UD1GA_Wolf_SecondPowerAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> Wolf_PowerAttack;
	
private:
	void FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void InMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void DashToTarget();
	void PerformWolfAttackTrace();
	void EndDash();

	FVector TargetLocation;
	FTimerHandle DashTimer;

	bool bSurvivorHit = false;
public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SecondWolfAnim(AD1KillerBase* Player, FName SectionName);

};
