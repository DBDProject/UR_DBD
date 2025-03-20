// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"
#include "D1GA_Dracula_PowerAttack.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GA_Dracula_PowerAttack : public UD1GameplayAbility
{
	GENERATED_BODY()

public:
	UD1GA_Dracula_PowerAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> WolfCooldownEffect;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge")
	TSubclassOf<UGameplayEffect> ChargeSlowEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cast")
	TSubclassOf<UGameplayEffect> CastSlowEffect;*/

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

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> TPV_PowerAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> FPV_PowerAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> Wolf_PowerAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EDraculaTransformationState CurrentTransformState;

private:
	void FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void WolfInMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void WolfSwingMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void CompleteCharge();

	bool bAttackHit = false;
	float ChargingStartTime = 0.0f;
	float ChargeDuration = 0.9f;
	float WolfChargeDuration = 0.85f;
	FTimerHandle ChargeTimerHandle;
public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DraculaPowerAttack(AD1KillerBase* Player, FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WolfPowerAttack(AD1KillerBase* Player, FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_WolfPowerAttackLoop(AD1KillerBase* Player, FName SectionName);
};
