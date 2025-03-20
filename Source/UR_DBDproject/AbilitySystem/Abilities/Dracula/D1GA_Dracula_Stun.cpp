// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Stun.h"

UD1GA_Dracula_Stun::UD1GA_Dracula_Stun(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_Stun::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_Stun::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	if (!Killer || !KillerController)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!TPV_Stun || !FPV_Stun)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 DestroyPallet Montage is NULL!"));
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

	Killer->GetController()->SetIgnoreLookInput(true);

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_DraculaStun(Killer);
	}

	// ✅ Montage 시작
	TPVAnimInstance->Montage_Play(TPV_Stun.Get());
	FPVAnimInstance->Montage_Play(FPV_Stun.Get());

	// ✅ Montage 끝나면 `OnEndMontage()` 실행
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_Stun::OnEndMontage);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_Stun.Get());
}

void UD1GA_Dracula_Stun::OnEndMontage(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_Stun::Multicast_DraculaStun_Implementation(AD1KillerBase* Player)
{
	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();

	TPVAnimInstance->Montage_Play(TPV_Stun.Get());
	FPVAnimInstance->Montage_Play(FPV_Stun.Get());
}

void UD1GA_Dracula_Stun::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	Killer->GetController()->SetIgnoreLookInput(false);
	UE_LOG(LogTemp, Log, TEXT("✅ Dracula_Stun GAS Finished"));
}