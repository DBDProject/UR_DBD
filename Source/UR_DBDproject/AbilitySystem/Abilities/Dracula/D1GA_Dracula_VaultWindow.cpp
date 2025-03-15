// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_VaultWindow.h"

UD1GA_Dracula_VaultWindow::UD1GA_Dracula_VaultWindow(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_VaultWindow::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_VaultWindow::ActivateAbility(
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
	if (!Killer || !KillerController)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	KillerController->SetIgnoreLookInput(true);

	VaultObj = Killer->GetVaultTarget();
	if (!VaultObj)
		return;
	VaultObj->MoveToVaultInteractionLocation(Killer);

	if (!TPV_VaultWindow || !FPV_VaultWindow)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 VaultWindow Montage is NULL!"));
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

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_VaultWindow(Killer);
	}

	TPVAnimInstance->Montage_Play(TPV_VaultWindow.Get());
	FPVAnimInstance->Montage_Play(FPV_VaultWindow.Get());

	FRotator LookAtRotation = (VaultObj->GetActorLocation() - Killer->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = 0.0f;
	KillerController->SetControlRotation(LookAtRotation);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_VaultWindow::OnEndMontage);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_VaultWindow.Get());
}

void UD1GA_Dracula_VaultWindow::OnEndMontage(UAnimMontage* Montage, bool bInterrupted)
{
	VaultObj = Killer->GetVaultTarget();
	if (!Montage)
		return;
	if (!Killer || !VaultObj) return;

	FVector StartLocation = VaultObj->GetStartPos();
	FVector TargetLocation = VaultObj->GetTargetPos();

	Killer->SetActorLocation(TargetLocation);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_VaultWindow::Multicast_VaultWindow_Implementation(AD1KillerBase* Player)
{
	if (!Player)
		return;

	UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();

	TPVAnimInstance->Montage_Play(TPV_VaultWindow.Get());
	FPVAnimInstance->Montage_Play(FPV_VaultWindow.Get());

}

void UD1GA_Dracula_VaultWindow::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	KillerController->SetIgnoreLookInput(false);
}