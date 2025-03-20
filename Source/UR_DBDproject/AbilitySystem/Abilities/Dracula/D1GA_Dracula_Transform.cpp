 // Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_Transform.h"
#include "D1Define.h"
#include "Net/UnrealNetwork.h"
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

	Killer = Cast<AD1KillerBase>(ActorInfo->AvatarActor.Get());
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	if (!Killer || !KillerController)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
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
	PrevTransformState = Killer->GetPrevTransformState();
	CurrentTransformState = Killer->GetCurrentTransformState();

	UE_LOG(LogTemp, Log, TEXT("✅ Transform Start - PrevState: %d, CurrentState: %d"),
		static_cast<int32>(PrevTransformState), static_cast<int32>(CurrentTransformState));

	if (HasAuthority(&ActivationInfo))
	{
		Multicast_OutTransform(Killer);
	}

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

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnOutMontageEnded);
	if (PrevTransformState == EDraculaTransformationState::Dracula)
		TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, Dracula_Transform.Get());
	else if (PrevTransformState == EDraculaTransformationState::Wolf)
		WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_Transform.Get());
	else if (PrevTransformState == EDraculaTransformationState::Bat)
		BatAnimInstance->Montage_SetEndDelegate(EndDelegate, Bat_Transform.Get());
}

void UD1GA_Dracula_Transform::OnOutMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
		return;
	UE_LOG(LogTemp, Log, TEXT("✅ Transform Montage Out Section Completed"));

	if (!Killer)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UAnimInstance* TPVAnimInstance = Killer->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Killer->GetFPVMesh()->GetAnimInstance();
	UAnimInstance* WolfAnimInstance = Killer->GetWolfMesh()->GetAnimInstance();
	UAnimInstance* BatAnimInstance = Killer->GetBatMesh()->GetAnimInstance();
	KillerController = Cast<AD1KillerController>(Killer->GetController());
	PrevTransformState = Killer->GetPrevTransformState();
	CurrentTransformState = Killer->GetCurrentTransformState();

	if (HasAuthority(&CurrentActivationInfo))
	{
		Multicast_EndTransform(Killer);
	}

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

			Killer->GetCharacterMesh()->bPauseAnims = true;
			Killer->GetFPVMesh()->bPauseAnims = true;
			Killer->GetWolfMesh()->bPauseAnims = false;
			Killer->GetBatMesh()->bPauseAnims = true;

			Killer->GetCharacterMesh()->SetComponentTickEnabled(false);
			Killer->GetFPVMesh()->SetComponentTickEnabled(false);
			Killer->GetWolfMesh()->SetComponentTickEnabled(true);
			Killer->GetBatMesh()->SetComponentTickEnabled(false);

			Killer->SwitchCamera(EDraculaTransformationState::Wolf);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Wolf Transform Montage (OutDracula -> InWolf)"));
			WolfAnimInstance->Montage_Play(Wolf_Transform.Get());
			WolfAnimInstance->Montage_JumpToSection(FName("InDracula"), Wolf_Transform.Get());

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Bat"));
			Killer->GetCharacterMesh()->SetHiddenInGame(true);
			Killer->GetFPVMesh()->SetHiddenInGame(true);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(false);

			Killer->GetCharacterMesh()->bPauseAnims = true;
			Killer->GetFPVMesh()->bPauseAnims = true;
			Killer->GetWolfMesh()->bPauseAnims = true;
			Killer->GetBatMesh()->bPauseAnims = false;

			Killer->GetCharacterMesh()->SetComponentTickEnabled(false);
			Killer->GetFPVMesh()->SetComponentTickEnabled(false);
			Killer->GetWolfMesh()->SetComponentTickEnabled(false);
			Killer->GetBatMesh()->SetComponentTickEnabled(true);

			Killer->SwitchCamera(EDraculaTransformationState::Bat);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Bat Transform Montage (OutDracula -> InBat)"));
			BatAnimInstance->Montage_Play(Bat_Transform.Get());
			BatAnimInstance->Montage_JumpToSection(FName("InDracula"), Bat_Transform.Get());

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			BatAnimInstance->Montage_SetEndDelegate(EndDelegate, Bat_Transform.Get());
		}
	}
	else if (PrevTransformState == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Wolf"));

		if (CurrentTransformState == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Dracula"));
			Killer->GetCharacterMesh()->SetHiddenInGame(false);
			Killer->GetCharacterMesh()->SetOwnerNoSee(true);
			Killer->GetFPVMesh()->SetHiddenInGame(false);
			Killer->GetFPVMesh()->SetOnlyOwnerSee(true);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(true);

			Killer->GetCharacterMesh()->bPauseAnims = false;
			Killer->GetFPVMesh()->bPauseAnims = false;
			Killer->GetWolfMesh()->bPauseAnims = true;
			Killer->GetBatMesh()->bPauseAnims = true;

			Killer->GetCharacterMesh()->SetComponentTickEnabled(true);
			Killer->GetFPVMesh()->SetComponentTickEnabled(true);
			Killer->GetWolfMesh()->SetComponentTickEnabled(false);
			Killer->GetBatMesh()->SetComponentTickEnabled(false);

			Killer->SwitchCamera(EDraculaTransformationState::Dracula);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Dracula Transform Montage (OutWolf -> InDracula)"));
			TPVAnimInstance->Montage_Play(Dracula_Transform.Get());
			TPVAnimInstance->Montage_JumpToSection(FName("InWolf"), Dracula_Transform.Get());

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, Dracula_Transform.Get());
		}
		else if (CurrentTransformState == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Bat"));
			Killer->GetCharacterMesh()->SetHiddenInGame(true);
			Killer->GetFPVMesh()->SetHiddenInGame(true);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(false);

			Killer->GetCharacterMesh()->bPauseAnims = true;
			Killer->GetFPVMesh()->bPauseAnims = true;
			Killer->GetWolfMesh()->bPauseAnims = true;
			Killer->GetBatMesh()->bPauseAnims = false;

			Killer->GetCharacterMesh()->SetComponentTickEnabled(false);
			Killer->GetFPVMesh()->SetComponentTickEnabled(false);
			Killer->GetWolfMesh()->SetComponentTickEnabled(false);
			Killer->GetBatMesh()->SetComponentTickEnabled(true);

			Killer->SwitchCamera(EDraculaTransformationState::Bat);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Bat Transform Montage (OutWolf -> InBat)"));
			BatAnimInstance->Montage_Play(Bat_Transform.Get());
			BatAnimInstance->Montage_JumpToSection(FName("InWolf"), Bat_Transform.Get());

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			BatAnimInstance->Montage_SetEndDelegate(EndDelegate, Bat_Transform.Get());
		}
	}
	else if (PrevTransformState == EDraculaTransformationState::Bat)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Bat"));

		if (CurrentTransformState == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Dracula"));
			Killer->GetCharacterMesh()->SetHiddenInGame(false);
			Killer->GetCharacterMesh()->SetOwnerNoSee(true);
			Killer->GetFPVMesh()->SetHiddenInGame(false);
			Killer->GetFPVMesh()->SetOnlyOwnerSee(true);
			Killer->GetWolfMesh()->SetHiddenInGame(true);
			Killer->GetBatMesh()->SetHiddenInGame(true);

			Killer->GetCharacterMesh()->bPauseAnims = false;
			Killer->GetFPVMesh()->bPauseAnims = false;
			Killer->GetWolfMesh()->bPauseAnims = true;
			Killer->GetBatMesh()->bPauseAnims = true;

			Killer->GetCharacterMesh()->SetComponentTickEnabled(true);
			Killer->GetFPVMesh()->SetComponentTickEnabled(true);
			Killer->GetWolfMesh()->SetComponentTickEnabled(false);
			Killer->GetBatMesh()->SetComponentTickEnabled(false);

			Killer->SwitchCamera(EDraculaTransformationState::Dracula);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Dracula Transform Montage (OutBat -> InDracula)"));
			TPVAnimInstance->Montage_Play(Dracula_Transform.Get());
			TPVAnimInstance->Montage_JumpToSection(FName("InBat"), Dracula_Transform.Get());

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			TPVAnimInstance->Montage_SetEndDelegate(EndDelegate, Dracula_Transform.Get());
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

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UD1GA_Dracula_Transform::OnFinalMontageEnded);
			WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_Transform.Get());
		}
	}

}

void UD1GA_Dracula_Transform::OnFinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UD1GA_Dracula_Transform::Multicast_OutTransform_Implementation(AD1KillerBase* Player)
{
	UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();
	UAnimInstance* WolfAnimInstance = Player->GetWolfMesh()->GetAnimInstance();
	UAnimInstance* BatAnimInstance = Player->GetBatMesh()->GetAnimInstance();

	EDraculaTransformationState PrevTransform = Player->GetPrevTransformState();
	EDraculaTransformationState CurrTransform = Player->GetCurrentTransformState();

	if (PrevTransform == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Playing Dracula_Transform Montage"));
		TPVAnimInstance->Montage_Play(Dracula_Transform.Get());

		if (CurrTransform == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutWolf Section"));
			TPVAnimInstance->Montage_JumpToSection(FName("OutWolf"), Dracula_Transform.Get());
		}
		else if (CurrTransform == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutBat Section"));
			TPVAnimInstance->Montage_JumpToSection(FName("OutBat"), Dracula_Transform.Get());
		}
	}
	else if (PrevTransform == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Playing Wolf_Transform Montage"));
		WolfAnimInstance->Montage_Play(Wolf_Transform.Get());

		if (CurrTransform == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutDracula Section"));
			WolfAnimInstance->Montage_JumpToSection(FName("OutDracula"), Wolf_Transform.Get());
		}
		else if (CurrTransform == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutBat Section"));
			WolfAnimInstance->Montage_JumpToSection(FName("OutBat"), Wolf_Transform.Get());
		}
	}
	else if (PrevTransform == EDraculaTransformationState::Bat)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Playing Bat_Transform Montage"));
		BatAnimInstance->Montage_Play(Bat_Transform.Get());

		if (CurrTransform == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutDracula Section"));
			BatAnimInstance->Montage_JumpToSection(FName("OutDracula"), Bat_Transform.Get());
		}
		else if (CurrTransform == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Jumping to OutWolf Section"));
			BatAnimInstance->Montage_JumpToSection(FName("OutWolf"), Bat_Transform.Get());
		}
	}
}

void UD1GA_Dracula_Transform::Multicast_EndTransform_Implementation(AD1KillerBase* Player)
{
	UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();
	UAnimInstance* WolfAnimInstance = Player->GetWolfMesh()->GetAnimInstance();
	UAnimInstance* BatAnimInstance = Player->GetBatMesh()->GetAnimInstance();

	EDraculaTransformationState PrevTransform = Player->GetPrevTransformState();
	EDraculaTransformationState CurrTransform = Player->GetCurrentTransformState();

	if (PrevTransform == EDraculaTransformationState::Dracula)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Dracula"));

		if (CurrTransform == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Wolf"));
			Multicast_SetHiddenState(Player, true, false, true);

			WolfAnimInstance->Montage_Play(Wolf_Transform.Get());
			WolfAnimInstance->Montage_JumpToSection(FName("InDracula"), Wolf_Transform.Get());
		}
		else if (CurrTransform == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Bat"));
			Multicast_SetHiddenState(Player, true, true, false);

			BatAnimInstance->Montage_Play(Bat_Transform.Get());
			BatAnimInstance->Montage_JumpToSection(FName("InDracula"), Bat_Transform.Get());
		}
	}
	else if (PrevTransform == EDraculaTransformationState::Wolf)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Wolf"));

		if (CurrTransform == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Dracula"));
			Multicast_SetHiddenState(Player, false, true, true);
			Player->GetCharacterMesh()->SetOwnerNoSee(true);
			Player->GetFPVMesh()->SetOnlyOwnerSee(true);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Dracula Transform Montage (OutWolf -> InDracula)"));
			TPVAnimInstance->Montage_Play(Dracula_Transform.Get());
			TPVAnimInstance->Montage_JumpToSection(FName("InWolf"), Dracula_Transform.Get());
		}
		else if (CurrTransform == EDraculaTransformationState::Bat)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Bat"));
			Multicast_SetHiddenState(Player, true, true, false);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Bat Transform Montage (OutWolf -> InBat)"));
			BatAnimInstance->Montage_Play(Bat_Transform.Get());
			BatAnimInstance->Montage_JumpToSection(FName("InWolf"), Bat_Transform.Get());
		}
	}
	else if (PrevTransform == EDraculaTransformationState::Bat)
	{
		UE_LOG(LogTemp, Log, TEXT("🎭 Transforming from Bat"));

		if (CurrTransform == EDraculaTransformationState::Dracula)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Dracula"));
			Multicast_SetHiddenState(Player, false, true, true);
			Player->GetCharacterMesh()->SetOwnerNoSee(true);
			Player->GetFPVMesh()->SetOnlyOwnerSee(true);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Dracula Transform Montage (OutBat -> InDracula)"));
			TPVAnimInstance->Montage_Play(Dracula_Transform.Get());
			TPVAnimInstance->Montage_JumpToSection(FName("InBat"), Dracula_Transform.Get());
		}
		else if (CurrTransform == EDraculaTransformationState::Wolf)
		{
			UE_LOG(LogTemp, Log, TEXT("🎯 Changing to Wolf"));
			Multicast_SetHiddenState(Player, true, false, true);

			UE_LOG(LogTemp, Log, TEXT("🎬 Playing Wolf Transform Montage (OutBat -> InWolf)"));
			WolfAnimInstance->Montage_Play(Wolf_Transform.Get());
			WolfAnimInstance->Montage_JumpToSection(FName("InBat"), Wolf_Transform.Get());
		}
	}
}

void UD1GA_Dracula_Transform::Multicast_SetHiddenState_Implementation(AD1KillerBase* Player, bool bDraculaVisible, bool bWolfVisible, bool bBatVisible)
{
	if (!Player) return;

	Player->GetCharacterMesh()->SetHiddenInGame(bDraculaVisible);
	Player->GetFPVMesh()->SetHiddenInGame(bDraculaVisible);
	Player->GetWolfMesh()->SetHiddenInGame(bWolfVisible);
	Player->GetBatMesh()->SetHiddenInGame(bBatVisible);

	Killer->GetCharacterMesh()->bPauseAnims = bDraculaVisible;
	Killer->GetFPVMesh()->bPauseAnims = bDraculaVisible;
	Killer->GetWolfMesh()->bPauseAnims = bWolfVisible;
	Killer->GetBatMesh()->bPauseAnims = bBatVisible;

	Killer->GetCharacterMesh()->SetComponentTickEnabled(!bDraculaVisible);
	Killer->GetFPVMesh()->SetComponentTickEnabled(!bDraculaVisible);
	Killer->GetWolfMesh()->SetComponentTickEnabled(!bWolfVisible);
	Killer->GetBatMesh()->SetComponentTickEnabled(!bWolfVisible);
}

void UD1GA_Dracula_Transform::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	KillerController->SetbTransform(false);
	UE_LOG(LogTemp, Log, TEXT("✅ Transform GAS END "));
}