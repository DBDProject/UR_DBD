// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_DestroyPallet.h"
#include "Interactables/D1Pallet.h"

UD1GA_Dracula_DestroyPallet::UD1GA_Dracula_DestroyPallet(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_DestroyPallet::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_DestroyPallet::ActivateAbility(
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

	if (!TPV_DestroyPallet || !FPV_DestroyPallet)
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

	AD1Pallet* Pallet = Killer->GetCurrentPallet();
	if (!Pallet || Pallet->GetCurrentState() != EPalletState::Down)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Pallet is NULL or is not Down!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	Killer->GetController()->SetIgnoreLookInput(true);
	EPalletLocation PalletLocation = Pallet->MovePlayerToInteractionPoint(Killer);

	FVector TargetLocation = Pallet->GetActorLocation();
	FVector KillerLocation = Killer->GetActorLocation();
	FRotator LookAtRotation = (TargetLocation - KillerLocation).Rotation();
	LookAtRotation.Pitch = -15.0f;
	KillerController->SetControlRotation(LookAtRotation);

	// ✅ Montage 시작
	TPVAnimInstance->Montage_Play(TPV_DestroyPallet.Get());
	FPVAnimInstance->Montage_Play(FPV_DestroyPallet.Get());

	// ✅ Montage 끝나면 `OnEndMontage()` 실행
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_DestroyPallet::OnEndMontage);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_DestroyPallet.Get());
}

void UD1GA_Dracula_DestroyPallet::OnEndMontage(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;
	UE_LOG(LogTemp, Log, TEXT("✅ DestroyPallet Montage Completed"));
	if (!Killer)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	AD1Pallet* Pallet = Killer->GetCurrentPallet();
	if (!Pallet)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	Pallet->OnDestroy();
	Pallet->SetCurrentState(EPalletState::Destroyed);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_DestroyPallet::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	Killer->GetController()->SetIgnoreLookInput(false);
	UE_LOG(LogTemp, Log, TEXT("✅ DestroyPallet GAS Finished"));	
}