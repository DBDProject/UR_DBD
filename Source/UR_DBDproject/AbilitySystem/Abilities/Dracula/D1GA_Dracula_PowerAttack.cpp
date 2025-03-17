// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_PowerAttack.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

UD1GA_Dracula_PowerAttack::UD1GA_Dracula_PowerAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_PowerAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}


	return true;
}

void UD1GA_Dracula_PowerAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CooldownEffect)
	{
		FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffect, GetAbilityLevel());
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpecHandle);
		UE_LOG(LogTemp, Log, TEXT("✅ Cooldown Effect Applied!"));
	}

	Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	if (!Killer || !KillerController)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!TPV_PowerAttack || !FPV_PowerAttack)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Montage is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();

	if (!TPVAnimInstance || !FPVAnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 AnimInstance is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	TPVAnimInstance->Montage_Play(TPV_PowerAttack.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PowerAttack.Get(), 1.0f);

	TPVAnimInstance->Montage_JumpToSection(FName("In"), TPV_PowerAttack.Get());
	FPVAnimInstance->Montage_JumpToSection(FName("In"), FPV_PowerAttack.Get());

	ChargingStartTime = GetWorld()->GetTimeSeconds();

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_DraculaPowerAttack(Killer, FName("In"));
	}

	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UD1GA_Dracula_PowerAttack::OnInputReleased);
		WaitInputReleaseTask->Activate();
	}
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
}


void UD1GA_Dracula_PowerAttack::OnInputReleased(float TimeHeld)
{
	float ChargingDuration = GetWorld()->GetTimeSeconds() - ChargingStartTime;

	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();

	TPVAnimInstance->Montage_Play(TPV_PowerAttack.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PowerAttack.Get(), 1.0f);

	if (ChargingDuration >= ChargeDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("🚨 차징 성공"));
		TPVAnimInstance->Montage_JumpToSection(FName("Out"), TPV_PowerAttack.Get());
		FPVAnimInstance->Montage_JumpToSection(FName("Out"), FPV_PowerAttack.Get()); 
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_DraculaPowerAttack(Killer, FName("Out"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("🚨 차징 실패"));
		TPVAnimInstance->Montage_JumpToSection(FName("Cancel"), TPV_PowerAttack.Get());
		FPVAnimInstance->Montage_JumpToSection(FName("Cancel"), FPV_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_DraculaPowerAttack(Killer, FName("Cancel"));
		}
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_PowerAttack::FinalMontageEnded);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::Multicast_DraculaPowerAttack_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();

	TPVAnimInstance->Montage_Play(TPV_PowerAttack.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PowerAttack.Get(), 1.0f);

	TPVAnimInstance->Montage_JumpToSection(SectionName, TPV_PowerAttack.Get());
	FPVAnimInstance->Montage_JumpToSection(SectionName, FPV_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_PowerAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("✅ Power Attack GAS END "));
}