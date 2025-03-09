 // Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Transform.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Killer/D1KillerController.h"

UD1GA_Dracula_Transform::UD1GA_Dracula_Transform(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_Transform::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}

	return true;
}

void UD1GA_Dracula_Transform::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("Dracula Transform Ability Activated!"));

	if (!HasAuthority(&ActivationInfo))
		return;

	AD1KillerBase* Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	if (!Killer)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!Dracula_Transform || !Wolf_Transform || !Bat_Transform)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();
	UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();
	UAnimInstance* BatAnimInstance = Killer->GetBatMesh()->GetAnimInstance();
	AD1KillerController* KillerController = Cast<AD1KillerController>(Killer->GetController());
	PrevTransformState = KillerController->GetPrevTransformState();
	CurrentTransformState = KillerController->GetCurrentTransformState();

	UE_LOG(LogTemp, Log, TEXT("✅ Transform Start - PrevState: %d, CurrentState: %d"),
		static_cast<int32>(PrevTransformState), static_cast<int32>(CurrentTransformState));

	if (PrevTransformState == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Playing Dracula_Transform Montage"));
		TPVAnimInstance->Montage_Play(Dracula_Transform.Get());

		if (CurrentTransformState == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutWolf Section"));
			TPVAnimInstance->Montage_JumpToSection(FName("OutWolf"), Dracula_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutBat Section"));
			TPVAnimInstance->Montage_JumpToSection(FName("OutBat"), Dracula_Transform.Get());
		}
	}
	else if (PrevTransformState == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Playing Wolf_Transform Montage"));
		WolfAnimInstance->Montage_Play(Wolf_Transform.Get());

		if (CurrentTransformState == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutDracula Section"));
			WolfAnimInstance->Montage_JumpToSection(FName("OutDracula"), Wolf_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutBat Section"));
			WolfAnimInstance->Montage_JumpToSection(FName("OutBat"), Wolf_Transform.Get());
		}
	}
	else if (PrevTransformState == EDraculaTransformationState::Bat)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Playing Bat_Transform Montage"));
		BatAnimInstance->Montage_Play(Bat_Transform.Get());

		if (CurrentTransformState == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutDracula Section"));
			BatAnimInstance->Montage_JumpToSection(FName("OutDracula"), Bat_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutWolf Section"));
			BatAnimInstance->Montage_JumpToSection(FName("OutWolf"), Bat_Transform.Get());
		}
	}


	FOnMontageBlendingOutStarted FinalBlendOutDelegate;
	FinalBlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnOutMontageEnded);
	if (PrevTransformState == EDraculaTransformationState::Dracula)
		TPVAnimInstance->Montage_SetBlendingOutDelegate(FinalBlendOutDelegate, Dracula_Transform.Get());
	else if (PrevTransformState == EDraculaTransformationState::Wolf)
		WolfAnimInstance->Montage_SetBlendingOutDelegate(FinalBlendOutDelegate, Wolf_Transform.Get());
	else if (PrevTransformState == EDraculaTransformationState::Bat)
		BatAnimInstance->Montage_SetBlendingOutDelegate(FinalBlendOutDelegate, Bat_Transform.Get());
}

void UD1GA_Dracula_Transform::OnOutMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;
	UE_LOG(LogTemp, Log, TEXT("✅ Transform Montage Out Section Completed"));
	AD1KillerBase* Killer = Cast<AD1KillerBase>(GetCurrentActorInfo()->AvatarActor.Get());
	if (!Killer)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();
	UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();
	UAnimInstance* BatAnimInstance = Killer->GetBatMesh()->GetAnimInstance();
	AD1KillerController* KillerController = Cast<AD1KillerController>(Killer->GetController());
	PrevTransformState = KillerController->GetPrevTransformState();
	CurrentTransformState = KillerController->GetCurrentTransformState();


	UE_LOG(LogTemp, Log, TEXT("✅ Transform Start - PrevState: %d, CurrentState: %d"),
		static_cast<int32>(PrevTransformState), static_cast<int32>(CurrentTransformState));

	if (PrevTransformState == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Dracula"));

		if (CurrentTransformState == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Wolf"));
			Killer->GetCharacterMesh()->SetHiddenInGame(true);
			Killer->GetFPVMesh()->SetHiddenInGame(true);
			Killer->GetWolfMesh()->SetHiddenInGame(false);
			Killer->GetBatMesh()->SetHiddenInGame(true);
			Killer->SwitchCamera(EDraculaTransformationState::Wolf);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Wolf Transform Montage (OutDracula -> InWolf)"));
			WolfAnimInstance->Montage_Play(Wolf_Transform.Get());
			WolfAnimInstance->Montage_JumpToSection(FName("InDracula"), Wolf_Transform.Get());

			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			WolfAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Wolf_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Bat"));
			Killer->GetCharacterMesh()->SetHiddenInGame(true);
			Killer->GetFPVMesh()->SetHiddenInGame(true);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(false);
			Killer->SwitchCamera(EDraculaTransformationState::Bat);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Bat Transform Montage (OutDracula -> InBat)"));
			BatAnimInstance->Montage_Play(Bat_Transform.Get());
			BatAnimInstance->Montage_JumpToSection(FName("InDracula"), Bat_Transform.Get());

			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			BatAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Bat_Transform.Get());
		}
	}
	else if (PrevTransformState == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Wolf"));

		if (CurrentTransformState == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Dracula"));
			Killer->GetCharacterMesh()->SetHiddenInGame(false);
			Killer->GetFPVMesh()->SetHiddenInGame(false);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(true);
			Killer->SwitchCamera(EDraculaTransformationState::Dracula);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Dracula Transform Montage (OutWolf -> InDracula)"));
			TPVAnimInstance->Montage_Play(Dracula_Transform.Get());
			TPVAnimInstance->Montage_JumpToSection(FName("InWolf"), Dracula_Transform.Get());

			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			TPVAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Dracula_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Bat"));
			Killer->GetCharacterMesh()->SetHiddenInGame(true);
			Killer->GetFPVMesh()->SetHiddenInGame(true);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(false);
			Killer->SwitchCamera(EDraculaTransformationState::Bat);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Bat Transform Montage (OutWolf -> InBat)"));
			BatAnimInstance->Montage_Play(Bat_Transform.Get());
			BatAnimInstance->Montage_JumpToSection(FName("InWolf"), Bat_Transform.Get());

			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			BatAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Bat_Transform.Get());
		}
	}
	else if (PrevTransformState == EDraculaTransformationState::Bat)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Bat"));

		if (CurrentTransformState == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Dracula"));
			Killer->GetCharacterMesh()->SetHiddenInGame(false);
			Killer->GetFPVMesh()->SetHiddenInGame(false);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(true);
			Killer->SwitchCamera(EDraculaTransformationState::Dracula);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Dracula Transform Montage (OutBat -> InDracula)"));
			TPVAnimInstance->Montage_Play(Dracula_Transform.Get());
			TPVAnimInstance->Montage_JumpToSection(FName("InBat"), Dracula_Transform.Get());

			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			TPVAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Dracula_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Wolf"));
			Killer->GetCharacterMesh()->SetHiddenInGame(true);
			Killer->GetFPVMesh()->SetHiddenInGame(true);
			Killer->GetWolfMesh()->SetHiddenInGame(false);
			Killer->GetBatMesh()->SetHiddenInGame(true);
			Killer->SwitchCamera(EDraculaTransformationState::Wolf);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Wolf Transform Montage (OutBat -> InWolf)"));
			WolfAnimInstance->Montage_Play(Wolf_Transform.Get());
			WolfAnimInstance->Montage_JumpToSection(FName("InBat"), Wolf_Transform.Get());

			FOnMontageBlendingOutStarted BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			WolfAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Wolf_Transform.Get());
		}
	}

}

void UD1GA_Dracula_Transform::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_Transform::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("✅ Transform GAS END "));
}