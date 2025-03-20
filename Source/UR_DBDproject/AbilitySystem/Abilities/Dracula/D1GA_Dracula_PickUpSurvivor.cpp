// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_PickUpSurvivor.h"
#include "Characters/Survivor/D1SurvivorBase.h"

UD1GA_Dracula_PickUpSurvivor::UD1GA_Dracula_PickUpSurvivor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_PickUpSurvivor::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_PickUpSurvivor::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
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

	if (!TPV_PickUpSurvivor || !FPV_PickUpSurvivor)
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

	AD1SurvivorBase* Survivor = Killer->GetDetectedCrawlSurvivor();
	if (!Survivor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_PickUpSurvivor(Killer);
	}

	//Survivor->TakePickUpFromKiller(Killer);
	Killer->SetCarriedSurvivor(Survivor);
	TPVAnimInstance->SetIsCarryingSurvivor(true);
	FPVAnimInstance->SetIsCarryingSurvivor(true);

	TPVAnimInstance->Montage_Play(TPV_PickUpSurvivor.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PickUpSurvivor.Get(), 1.0f);

	FRotator LookAtRotation = (Survivor->GetActorLocation() - Killer->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = -60.0f;
	KillerController->SetControlRotation(LookAtRotation);

	TPVAnimInstance->SetIsCarryingSurvivor(true);
	FPVAnimInstance->SetIsCarryingSurvivor(true);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_PickUpSurvivor::OnPickUpMontageEnded);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_PickUpSurvivor.Get());
}

void UD1GA_Dracula_PickUpSurvivor::OnPickUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;

	KillerController->SetIgnoreLookInput(false);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_PickUpSurvivor::Multicast_PickUpSurvivor_Implementation(AD1KillerBase* Player)
{
	if (!Player) return;

	UD1KillerBaseAnim* TPVAnimInstance = Cast<UD1KillerBaseAnim>(Player->GetCharacterMesh().Get()->GetAnimInstance());
	UD1KillerBaseAnim* FPVAnimInstance = Cast<UD1KillerBaseAnim>(Player->GetFPVMesh().Get()->GetAnimInstance());

	TPVAnimInstance->Montage_Play(TPV_PickUpSurvivor.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PickUpSurvivor.Get(), 1.0f);

	AD1SurvivorBase* Survivor = Player->GetDetectedCrawlSurvivor();

	Survivor->TakePickUpFromKiller(Player);

}

void UD1GA_Dracula_PickUpSurvivor::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("🚨 PickUp Ability Ended!"));
}