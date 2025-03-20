// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/Wolf/D1GA_Wolf_VaultWindow.h"
#include "Components/CapsuleComponent.h"

UD1GA_Wolf_VaultWindow::UD1GA_Wolf_VaultWindow(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Wolf_VaultWindow::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Wolf_VaultWindow::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("Wolf VaultWindow Ability Activated!"));

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

	if (!Wolf_VaultWindow)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 VaultWindow Montage is NULL!"));
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
		Multicast_WolfVaultWindow(Killer);
	}

	Killer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
	//WolfAnimInstance->Montage_Play(Wolf_VaultWindow.Get());
	Killer->PlayAnimMontage(Wolf_VaultWindow, 1.0f);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Wolf_VaultWindow::OnEndMontage);
	WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_VaultWindow.Get());
}

void UD1GA_Wolf_VaultWindow::OnEndMontage(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(FGameplayAbilitySpecHandle(), nullptr, FGameplayAbilityActivationInfo(), true, false);
}

void UD1GA_Wolf_VaultWindow::Multicast_WolfVaultWindow_Implementation(AD1KillerBase* Player)
{
	Player->PlayAnimMontage(Wolf_VaultWindow, 1.0f);
}

void UD1GA_Wolf_VaultWindow::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (!Killer || !KillerController)
		return;
	Killer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	KillerController->SetIgnoreLookInput(false);
}