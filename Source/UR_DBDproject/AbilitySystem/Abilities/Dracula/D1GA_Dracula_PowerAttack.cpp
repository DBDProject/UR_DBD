// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_PowerAttack.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interactables/D1Pallet.h"


UD1GA_Dracula_PowerAttack::UD1GA_Dracula_PowerAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_PowerAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}


	return true;
}

void UD1GA_Dracula_PowerAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CooldownEffect)
	{
		FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffect, GetAbilityLevel());
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpecHandle);
		UE_LOG(LogTemp, Log, TEXT("✅ Cooldown Effect Applied!"));
	}

	Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	if (!Killer || !KillerController)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	CurrentTransformState = Killer->GetCurrentTransformState();

	if (CurrentTransformState == EDraculaTransformationState::Dracula)
	{
		if (!TPV_PowerAttack || !FPV_PowerAttack)
		{
			UE_LOG(LogTemp, Error, TEXT("🚨 Montage is NULL!"));
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

		TPVAnimInstance->Montage_Play(TPV_PowerAttack.Get(), 1.0f);
		FPVAnimInstance->Montage_Play(FPV_PowerAttack.Get(), 1.0f);

		TPVAnimInstance->Montage_JumpToSection(FName("In"), TPV_PowerAttack.Get());
		FPVAnimInstance->Montage_JumpToSection(FName("In"), FPV_PowerAttack.Get());

		Killer->GetCharacterMovement()->MaxWalkSpeed = 380.0f;

		ChargingStartTime = GetWorld()->GetTimeSeconds();

		if (HasAuthority(&ActivationInfo))
		{
			Multicast_DraculaPowerAttack(Killer, FName("In"));
		}

		UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
		if (WaitInputReleaseTask)
		{
			WaitInputReleaseTask->OnRelease.AddDynamic(this, &UD1GA_Dracula_PowerAttack::OnInputReleased);
			WaitInputReleaseTask->Activate();
		}
	}
	else if (CurrentTransformState == EDraculaTransformationState::Wolf)
	{
		if (!Wolf_PowerAttack)
		{
			UE_LOG(LogTemp, Error, TEXT("🚨 Montage is NULL!"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();
		if (!WolfAnimInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("🚨 AnimInstance is NULL!"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

		WolfAnimInstance->Montage_JumpToSection(FName("In"), Wolf_PowerAttack.Get());

		if (HasAuthority(&ActivationInfo))
		{
			Multicast_WolfPowerAttack(Killer, FName("In"));
		}

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UD1GA_Dracula_PowerAttack::WolfInMontageEnded);
		WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_PowerAttack.Get());

	}
	
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
}


void UD1GA_Dracula_PowerAttack::OnInputReleased(float TimeHeld)
{
	float ChargingDuration = GetWorld()->GetTimeSeconds() - ChargingStartTime;

	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();

	TPVAnimInstance->Montage_Play(TPV_PowerAttack.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PowerAttack.Get(), 1.0f);

	if (ChargingDuration >= ChargeDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("🚨 차징 성공"));
		TPVAnimInstance->Montage_JumpToSection(FName("Out"), TPV_PowerAttack.Get());
		FPVAnimInstance->Montage_JumpToSection(FName("Out"), FPV_PowerAttack.Get());
		Killer->GetCharacterMovement()->MaxWalkSpeed = 200.0f;
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_DraculaPowerAttack(Killer, FName("Out"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("🚨 차징 실패"));
		TPVAnimInstance->Montage_JumpToSection(FName("Cancel"), TPV_PowerAttack.Get());
		FPVAnimInstance->Montage_JumpToSection(FName("Cancel"), FPV_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_DraculaPowerAttack(Killer, FName("Cancel"));
		}
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_PowerAttack::FinalMontageEnded);
	TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, TPV_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::Multicast_DraculaPowerAttack_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();

	TPVAnimInstance->Montage_Play(TPV_PowerAttack.Get(), 1.0f);
	FPVAnimInstance->Montage_Play(FPV_PowerAttack.Get(), 1.0f);

	TPVAnimInstance->Montage_JumpToSection(SectionName, TPV_PowerAttack.Get());
	FPVAnimInstance->Montage_JumpToSection(SectionName, FPV_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::Multicast_WolfPowerAttack_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* WolfAnimInstance = Player->GetWolfMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

	WolfAnimInstance->Montage_JumpToSection(SectionName, Wolf_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::CompleteCharge()
{
	UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

	WolfAnimInstance->Montage_JumpToSection(FName("Swing"), TPV_PowerAttack.Get());

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_PowerAttack::WolfSwingMontageEnded);
	WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::WolfSwingMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;

	UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();

	bAttackHit = Killer->GetbSurvivorHit();

	AD1Pallet* Pallet = Killer->GetCurrentPallet();
	if (Pallet)
	{
		if (Pallet->GetCurrentState() != EPalletState::Down)
		{
			Pallet = nullptr;
		}
	}

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get());
	if (bAttackHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Hit!"));
		WolfAnimInstance->Montage_JumpToSection(FName("Hit"), Wolf_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_WolfPowerAttack(Killer, FName("Hit"));
		}
	}
	else if (Pallet != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Wipe!"));
		WolfAnimInstance->Montage_JumpToSection(FName("Wipe"), Wolf_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_WolfPowerAttack(Killer, FName("Wipe"));
		}

		Pallet->OnDestroy();
		Pallet->SetCurrentState(EPalletState::Destroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wolf Attack Miss!"));
		WolfAnimInstance->Montage_JumpToSection(FName("Miss"), Wolf_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_WolfPowerAttack(Killer, FName("Miss"));
		}
	}

	FOnMontageEnded FinalEndDelegate;
	FinalEndDelegate.BindUObject(this, &UD1GA_Dracula_PowerAttack::FinalMontageEnded);
	WolfAnimInstance->Montage_SetEndDelegate(FinalEndDelegate, Wolf_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_PowerAttack::WolfInMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

	WolfAnimInstance->Montage_Pause(Wolf_PowerAttack.Get());

	if (HasAuthority(&CurrentActivationInfo))
	{
		Multicast_WolfPowerAttackLoop(Killer, FName("In"));
	}

	GetWorld()->GetTimerManager().SetTimer(ChargeTimerHandle, this, &UD1GA_Dracula_PowerAttack::CompleteCharge, 0.85f, false);
}

void UD1GA_Dracula_PowerAttack::Multicast_WolfPowerAttackLoop_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* WolfAnimInstance = Player->GetWolfMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

	WolfAnimInstance->Montage_Pause(Wolf_PowerAttack.Get());
}

void UD1GA_Dracula_PowerAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	Killer->GetCharacterMovement()->MaxWalkSpeed = 460.0f;
	UE_LOG(LogTemp, Log, TEXT("✅ Power Attack GAS END "));
}