// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/Wolf/D1GA_Wolf_Attack.h"

UD1GA_Wolf_Attack::UD1GA_Wolf_Attack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Wolf_Attack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Wolf_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UE_LOG(LogTemp, Log, TEXT("Wolf Attack Ability Activated!"));
	
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
	
	if (!Wolf_Attack)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Attack Montage is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	UAnimInstance* WolfAnimInstance = Killer->GetMesh()->GetAnimInstance();
	if (!WolfAnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 AnimInstance is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_WolfAttack(Killer, FName("In"));
	}

	WolfAnimInstance->Montage_Play(Wolf_Attack.Get());
	WolfAnimInstance->Montage_JumpToSection(FName("In"), Wolf_Attack.Get());


	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Wolf_Attack::OnInMontageEnded);
	WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_Attack.Get());
}

void UD1GA_Wolf_Attack::OnInMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Montage Interrupted!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Montage Ended!"));
	}

	if (!Montage)
		return;

	UAnimInstance* WolfAnimInstance = Killer->GetMesh()->GetAnimInstance();

	bAttackHit = Killer->GetbSurvivorHit();

	WolfAnimInstance->Montage_Play(Wolf_Attack.Get());
	if (bAttackHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Hit!"));
		WolfAnimInstance->Montage_JumpToSection(FName("Hit"), Wolf_Attack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_WolfAttack(Killer, FName("Hit"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Miss!"));
		WolfAnimInstance->Montage_JumpToSection(FName("Miss"), Wolf_Attack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_WolfAttack(Killer, FName("Miss"));
		}
	}

	FOnMontageEnded FinalEndDelegate;
	FinalEndDelegate.BindUObject(this, &UD1GA_Wolf_Attack::OnFinalMontageEnded);
	WolfAnimInstance->Montage_SetEndDelegate(FinalEndDelegate, Wolf_Attack.Get());
}

void UD1GA_Wolf_Attack::Multicast_WolfAttack_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* WolfAnimInstance = Player->GetMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_Attack.Get(), 1.0f);

	WolfAnimInstance->Montage_JumpToSection(SectionName, Wolf_Attack.Get());
}

void UD1GA_Wolf_Attack::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UD1GA_Wolf_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	bAttackHit = false;
	KillerController->SetIgnoreLookInput(false);
	UE_LOG(LogTemp, Log, TEXT("✅ Wolf Attack GAS END "));
}