// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Attack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Characters/Killer/D1KillerBase.h"

UD1GA_Dracula_Attack::UD1GA_Dracula_Attack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}

	return true;
}

void UD1GA_Dracula_Attack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    //UE_LOG(LogTemp, Log, TEXT("Dracula Attack Ability Activated!"));

    if (!HasAuthority(&ActivationInfo))
        return;

    AD1KillerBase* Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
    if (!Killer)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    if (!TPV_Attack || !FPV_Attack)
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

    // ✅ "In" 섹션 실행
    TPVAnimInstance->Montage_Play(TPV_Attack.Get(), 1.0f);
    FPVAnimInstance->Montage_Play(FPV_Attack.Get(), 1.0f);

    TPVAnimInstance->Montage_JumpToSection(FName("In"), TPV_Attack.Get());
    FPVAnimInstance->Montage_JumpToSection(FName("In"), FPV_Attack.Get());

    // ✅ "In" 섹션이 끝나면 `OnInMontageEnded()` 실행
    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Attack::OnInMontageEnded);
    TPVAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, TPV_Attack.Get());
}

void UD1GA_Dracula_Attack::OnInMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage)
        return;

    //UE_LOG(LogTemp, Log, TEXT("✅ 'In' Section Completed - Jumping to Hit or Miss"));

    AD1KillerBase* Killer = Cast<AD1KillerBase>(GetCurrentActorInfo()->AvatarActor.Get());
    if (!Killer) return;

    UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
    UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();

    if (!TPVAnimInstance || !FPVAnimInstance)
        return;

    TPVAnimInstance->Montage_Play(TPV_Attack.Get());
    FPVAnimInstance->Montage_Play(FPV_Attack.Get());

    
    // ✅ 적중 여부에 따라 "Hit" 또는 "Miss"로 이동
    if (bAttackHit)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ bAttackHit = true"));
        TPVAnimInstance->Montage_JumpToSection(FName("Hit"), TPV_Attack.Get());
        FPVAnimInstance->Montage_JumpToSection(FName("Hit"), FPV_Attack.Get());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✅ bAttackHit = false"));
        TPVAnimInstance->Montage_JumpToSection(FName("Miss"), TPV_Attack.Get());
        FPVAnimInstance->Montage_JumpToSection(FName("Miss"), FPV_Attack.Get());
    }

    // ✅ "Hit" 또는 "Miss"가 끝나면 `OnFinalMontageEnded()` 실행
    FOnMontageBlendingOutStarted FinalBlendOutDelegate;
    FinalBlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Attack::OnFinalMontageEnded);
    TPVAnimInstance->Montage_SetBlendingOutDelegate(FinalBlendOutDelegate, TPV_Attack.Get());
}

void UD1GA_Dracula_Attack::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    //UE_LOG(LogTemp, Log, TEXT("✅ Attack Montage Completed - Ending Ability"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UD1GA_Dracula_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled); 
    
	bAttackHit = false;
    UE_LOG(LogTemp, Log, TEXT("✅ Attack GAS END "));
}
