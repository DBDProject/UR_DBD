// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Attack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

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


    Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
    KillerController = Cast<AD1KillerController>(Killer->GetController());
    if (!Killer || !KillerController)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    KillerController->SetIgnoreLookInput(true);

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

    if (HasAuthority(&ActivationInfo))
    {
        Multicast_PlayAttack(Killer, FName("In"));
    }

    TPVAnimInstance->Montage_Play(TPV_Attack.Get(), 1.0f);
    FPVAnimInstance->Montage_Play(FPV_Attack.Get(), 1.0f);

    TPVAnimInstance->Montage_JumpToSection(FName("In"), TPV_Attack.Get());
    FPVAnimInstance->Montage_JumpToSection(FName("In"), FPV_Attack.Get());

    // ✅ "In" 섹션이 끝나면 `OnInMontageEnded()` 실행
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UD1GA_Dracula_Attack::OnInMontageEnded);
    TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_Attack.Get());
}

void UD1GA_Dracula_Attack::OnInMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage)
        return;

    //UE_LOG(LogTemp, Log, TEXT("✅ 'In' Section Completed - Jumping to Hit or Miss"));

    if (!Killer) return;

    UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
    UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();

    if (!TPVAnimInstance || !FPVAnimInstance)
        return;

    TPVAnimInstance->Montage_Play(TPV_Attack.Get());
    FPVAnimInstance->Montage_Play(FPV_Attack.Get());

    bAttackHit = Killer->GetbSurvivorHit();
    
    // ✅ 적중 여부에 따라 "Hit" 또는 "Miss"로 이동
    if (bAttackHit)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ 공격 성공 (Hit)"));
        TPVAnimInstance->Montage_JumpToSection(FName("Hit"), TPV_Attack.Get());
        FPVAnimInstance->Montage_JumpToSection(FName("Hit"), FPV_Attack.Get());
        if (HasAuthority(&CurrentActivationInfo))
        {
            Multicast_PlayAttack(Killer, FName("Hit"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("❌ 공격 실패 (Miss)"));
        TPVAnimInstance->Montage_JumpToSection(FName("Miss"), TPV_Attack.Get());
        FPVAnimInstance->Montage_JumpToSection(FName("Miss"), FPV_Attack.Get());
        if (HasAuthority(&CurrentActivationInfo))
        {
            Multicast_PlayAttack(Killer, FName("Miss"));
        }
    }

    // ✅ "Hit" 또는 "Miss"가 끝나면 `OnFinalMontageEnded()` 실행
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UD1GA_Dracula_Attack::OnFinalMontageEnded);
    TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_Attack.Get());
}

void UD1GA_Dracula_Attack::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    //UE_LOG(LogTemp, Log, TEXT("✅ Attack Montage Completed - Ending Ability"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_Attack::Multicast_PlayAttack_Implementation(AD1KillerBase* Player, FName SectionName)
{
    UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
    UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();

    TPVAnimInstance->Montage_Play(TPV_Attack.Get(), 1.0f);
    FPVAnimInstance->Montage_Play(FPV_Attack.Get(), 1.0f);

    TPVAnimInstance->Montage_JumpToSection(SectionName, TPV_Attack.Get());
    FPVAnimInstance->Montage_JumpToSection(SectionName, FPV_Attack.Get());
}


void UD1GA_Dracula_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled); 
    
    bAttackHit = false;
    KillerController->SetIgnoreLookInput(false);
    UE_LOG(LogTemp, Log, TEXT("✅ Attack GAS END "));
}
