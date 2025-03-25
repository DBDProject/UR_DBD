// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/Wolf/D1GA_Wolf_Stun.h"
#include "Interactables/D1Pallet.h"

UD1GA_Wolf_Stun::UD1GA_Wolf_Stun(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Wolf_Stun::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Wolf_Stun::ActivateAbility(
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

	if (!Wolf_Stun)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 DestroyPallet Montage is NULL!"));
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

	Killer->GetController()->SetIgnoreLookInput(true);

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_WolfStun(Killer);
	}

	// ✅ Montage 시작
	WolfAnimInstance->Montage_Play(Wolf_Stun.Get());

	// ✅ Montage 끝나면 `OnEndMontage()` 실행
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Wolf_Stun::OnEndMontage);
	WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_Stun.Get());
}

void UD1GA_Wolf_Stun::OnEndMontage(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Wolf_Stun::Multicast_WolfStun_Implementation(AD1KillerBase* Player)
{
	AD1Pallet* Pallet = Player->GetCurrentPallet();

	EPalletLocation Location = Pallet->FindClosestInteractionPoint(Player);

	Pallet->MovePlayerToInteractionPoint(Player, ECharacterType::DRACULA);

	UAnimInstance* WolfAnimInstance = Player->GetMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_Stun.Get());
}

void UD1GA_Wolf_Stun::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	Killer->GetController()->SetIgnoreLookInput(false);
	UE_LOG(LogTemp, Log, TEXT("✅ Dracula_Stun GAS Finished"));
}