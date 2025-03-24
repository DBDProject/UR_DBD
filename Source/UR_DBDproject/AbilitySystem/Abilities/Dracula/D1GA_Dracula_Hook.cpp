// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Hook.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Animation/D1KillerBaseAnim.h"

UD1GA_Dracula_Hook::UD1GA_Dracula_Hook(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_Hook::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_Hook::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!HasAuthority(&ActivationInfo))
		return;
	
	Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	if (!Killer)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	KillerController->SetIgnoreLookInput(true);

	Hook = Killer->GetCurrentHook();
	AD1SurvivorBase* CarriedSurvivor = Killer->GetCarriedSurvivor();
	if (!Hook || !CarriedSurvivor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!TPV_HookSurvivor || !FPV_HookSurvivor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UD1KillerBaseAnim* TPVAnimInstance = Cast<UD1KillerBaseAnim>(Killer->GetCharacterMesh().Get()->GetAnimInstance());
	UD1KillerBaseAnim* FPVAnimInstance = Cast<UD1KillerBaseAnim>(Killer->GetFPVMesh().Get()->GetAnimInstance());
	if (!TPVAnimInstance || !FPVAnimInstance)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_HookSurvivor(Killer);
	}

	MoveToHookLocation();
	CarriedSurvivor->StartOnHooked(Hook);

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	TPVAnimInstance->Montage_Play(TPV_HookSurvivor.Get());
	FPVAnimInstance->Montage_Play(FPV_HookSurvivor.Get());

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_Hook::OnFinalMontageEnded);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_HookSurvivor.Get());
}

void UD1GA_Dracula_Hook::MoveToHookLocation()
{
	FVector HookLocation = Hook->GetActorLocation();
	FVector ForwardVector = Hook->GetActorForwardVector();

	FVector TargetLocation = HookLocation + ForwardVector * 300.0f;
	TargetLocation.Z += 120.0f;

	KillerController->GetPawn()->SetActorLocation(TargetLocation);
	FRotator LookAtRotation = (HookLocation - TargetLocation).Rotation();
	LookAtRotation.Pitch = 15.0f;
	KillerController->SetControlRotation(LookAtRotation);
}

void UD1GA_Dracula_Hook::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;

	UD1KillerBaseAnim* TPVAnimInstance = Cast<UD1KillerBaseAnim>(Killer->GetCharacterMesh().Get()->GetAnimInstance());
	UD1KillerBaseAnim* FPVAnimInstance = Cast<UD1KillerBaseAnim>(Killer->GetFPVMesh().Get()->GetAnimInstance());

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	Killer->SetCarriedSurvivor(nullptr);
	Cast<AD1KillerBase>(KillerController->GetPawn())->SetDetectedCrawlSurvivor(nullptr);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_Hook::Multicast_HookSurvivor_Implementation(AD1KillerBase* Player)
{
	UD1KillerBaseAnim* TPVAnimInstance = Cast<UD1KillerBaseAnim>(Player->GetCharacterMesh().Get()->GetAnimInstance());
	UD1KillerBaseAnim* FPVAnimInstance = Cast<UD1KillerBaseAnim>(Player->GetFPVMesh().Get()->GetAnimInstance());

	TPVAnimInstance->SetIsCarryingSurvivor(false);
	FPVAnimInstance->SetIsCarryingSurvivor(false);

	TPVAnimInstance->Montage_Play(TPV_HookSurvivor.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_HookSurvivor.Get(), 1.0f);
}

void UD1GA_Dracula_Hook::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	KillerController->SetIgnoreLookInput(false);
	UE_LOG(LogTemp, Log, TEXT("Dracula Hook Ability Ended!"));
}