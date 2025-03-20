// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Carry_Attack.h"

UD1GA_Dracula_Carry_Attack::UD1GA_Dracula_Carry_Attack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_Carry_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}

	return true;
}

void UD1GA_Dracula_Carry_Attack::ActivateAbility(
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

    KillerController->SetIgnoreLookInput(true);

    if (!TPV_CarryAttack || !FPV_CarryAttack)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 Attack Montage is NULL!"));
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
        Multicast_PlayAttack(Killer, FName("In"));
    }

    TPVAnimInstance->Montage_Play(TPV_CarryAttack.Get(), 1.0f);
    FPVAnimInstance->Montage_Play(FPV_CarryAttack.Get(), 1.0f);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UD1GA_Dracula_Carry_Attack::OnFinalMontageEnded);
    TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_CarryAttack.Get());
}

void UD1GA_Dracula_Carry_Attack::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_Carry_Attack::Multicast_PlayAttack_Implementation(AD1KillerBase* Player, FName SectionName)
{
    UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
    UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();

    TPVAnimInstance->Montage_Play(TPV_CarryAttack.Get(), 1.0f);
    FPVAnimInstance->Montage_Play(FPV_CarryAttack.Get(), 1.0f);
}

void UD1GA_Dracula_Carry_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    KillerController->SetIgnoreLookInput(false);
    UE_LOG(LogTemp, Log, TEXT("✅ Attack GAS END "));
}
