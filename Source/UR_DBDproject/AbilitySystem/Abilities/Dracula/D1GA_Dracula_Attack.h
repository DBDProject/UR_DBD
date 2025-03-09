// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"
#include "D1GA_Dracula_Attack.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GA_Dracula_Attack : public UD1GameplayAbility
{
	GENERATED_BODY()

public:
	UD1GA_Dracula_Attack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
	TObjectPtr<class UAnimMontage> TPV_Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> FPV_Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bAttackHit = false;

private:
	void OnInMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	void SetAttackHit(bool bHit) { bAttackHit = bHit; }
};
