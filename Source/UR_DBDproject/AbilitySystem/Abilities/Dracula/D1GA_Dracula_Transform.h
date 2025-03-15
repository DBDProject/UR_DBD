// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"

#include "D1GA_Dracula_Transform.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GA_Dracula_Transform : public UD1GameplayAbility
{
	GENERATED_BODY()

public:
	UD1GA_Dracula_Transform(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


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
	TObjectPtr<class UAnimMontage> Dracula_Transform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> Wolf_Transform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> Bat_Transform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EDraculaTransformationState PrevTransformState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EDraculaTransformationState CurrentTransformState;

public:
	void OnOutMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OutTransform(AD1KillerBase* Player);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndTransform(AD1KillerBase* Player);

};
