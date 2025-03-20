// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_DropSurvivor.h"
#include "Characters/Survivor/D1SurvivorBase.h"

UD1GA_Dracula_DropSurvivor::UD1GA_Dracula_DropSurvivor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_DropSurvivor::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_DropSurvivor::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	if (!Killer)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	KillerController->SetIgnoreLookInput(true);

	if (!TPV_DropSurvivor || !FPV_DropSurvivor)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 PickUpSurvivor Montage is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	UD1KillerBaseAnim* TPVAnimInstance = Cast<UD1KillerBaseAnim>(Killer->GetCharacterMesh().Get()->GetAnimInstance());
	UD1KillerBaseAnim* FPVAnimInstance = Cast<UD1KillerBaseAnim>(Killer->GetFPVMesh().Get()->GetAnimInstance());
	if (!TPVAnimInstance || !FPVAnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 AnimInstance is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AD1SurvivorBase* Survivor = Killer->GetCarriedSurvivor();
	if (!Survivor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_DropSurvivor(Killer);
	}

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	TPVAnimInstance->Montage_Play(TPV_DropSurvivor.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_DropSurvivor.Get(), 1.0f);

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	Survivor->TakeDropFromKiller(Killer);
	Killer->SetCarriedSurvivor(nullptr);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_DropSurvivor::OnDropMontageEnded);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_DropSurvivor.Get());
}

void UD1GA_Dracula_DropSurvivor::Multicast_DropSurvivor_Implementation(AD1KillerBase* Player)
{
	if (!Player) return;

	UD1KillerBaseAnim* TPVAnimInstance = Cast<UD1KillerBaseAnim>(Player->GetCharacterMesh().Get()->GetAnimInstance());
	UD1KillerBaseAnim* FPVAnimInstance = Cast<UD1KillerBaseAnim>(Player->GetFPVMesh().Get()->GetAnimInstance());

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	TPVAnimInstance->Montage_Play(TPV_DropSurvivor.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_DropSurvivor.Get(), 1.0f);

}

void UD1GA_Dracula_DropSurvivor::OnDropMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;

	KillerController->SetIgnoreLookInput(false);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_DropSurvivor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("🚨 Drop Ability Ended!"));
}