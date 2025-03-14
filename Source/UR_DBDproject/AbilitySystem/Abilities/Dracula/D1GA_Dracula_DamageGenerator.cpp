// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_DamageGenerator.h"
#include "Interactables/D1Generator.h"
#include "Kismet/KismetMathLibrary.h"

UD1GA_Dracula_DamageGenerator::UD1GA_Dracula_DamageGenerator(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Dracula_DamageGenerator::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}
	return true;
}

void UD1GA_Dracula_DamageGenerator::ActivateAbility(
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
	KillerController->SetIgnoreLookInput(true);

	if (!TPV_DamageGenerator || !FPV_DamageGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 DamageGenerator Montage is NULL!"));
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

	AD1Generator* Generator = Killer->GetCurrentGenerator();
	if (!Generator || Generator->GetRepairProgress() == 0.0f || Generator->GetRepairProgress() >= 100.0f
		|| Generator->GetCurrentState() == EGeneratorState::Breaking)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	EGeneratorInteractionPosition Pos =
		Generator->FindInteractionPosition(Killer);
	
	if (HasAuthority(&ActivationInfo))
	{
		Multicast_PlayDamageGenerator(Killer);
	}

	MoveToGeneratorPosition(Pos, Killer);

	// 플레이어 방향을 발전기로 조정 (자동 회전)
	FRotator LookAtRotation = (Generator->GetActorLocation() - Killer->GetActorLocation()).Rotation();
	LookAtRotation.Pitch = -15.0f;
	KillerController->SetControlRotation(LookAtRotation);

	Generator->SetCurrentState(EGeneratorState::Breaking);

	TPVAnimInstance->Montage_Play(TPV_DamageGenerator.Get());
	FPVAnimInstance->Montage_Play(FPV_DamageGenerator.Get());

	FOnMontageBlendingOutStarted FinalBlendOutDelegate;
	FinalBlendOutDelegate.BindUObject(this, &UD1GA_Dracula_DamageGenerator::OnEndMontage);
	TPVAnimInstance->Montage_SetBlendingOutDelegate(FinalBlendOutDelegate, TPV_DamageGenerator.Get());
}

void UD1GA_Dracula_DamageGenerator::OnEndMontage(UAnimMontage* Montage, bool bInterrupted)
{
	if (Killer)
	{
		AD1Generator* Generator = Killer->GetCurrentGenerator();
		if (Generator)
		{
			Generator->OnDamage();
			Generator->SetCurrentState(EGeneratorState::Idle);
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Dracula_DamageGenerator::Multicast_PlayDamageGenerator_Implementation(AD1KillerBase* Player)
{
	if (!Player) return;

	EGeneratorInteractionPosition Pos = Player->GetCurrentGenerator()->FindInteractionPosition(Player);
	MoveToGeneratorPosition(Pos, Player);

	UAnimInstance* TPVAnimInstance = Player->GetCharacterMesh()->GetAnimInstance();
	UAnimInstance* FPVAnimInstance = Player->GetFPVMesh()->GetAnimInstance();
	if (!TPVAnimInstance || !FPVAnimInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 AnimInstance is NULL!"));
		return;
	}
	TPVAnimInstance->Montage_Play(TPV_DamageGenerator.Get());
	FPVAnimInstance->Montage_Play(FPV_DamageGenerator.Get());
}

void UD1GA_Dracula_DamageGenerator::MoveToGeneratorPosition(EGeneratorInteractionPosition Position, AD1KillerBase* Player)
{
	AD1Generator* Generator = Player->GetCurrentGenerator();
	FVector GeneratorLocation = Generator->GetActorLocation();
	FVector ForwardVector = Generator->GetActorForwardVector();
	FVector RightVector = Generator->GetActorRightVector();
	FVector TargetLocation;

	// 플레이어를 발전기 위치로 이동
	switch (Position)
	{
	case EGeneratorInteractionPosition::Front:
		TargetLocation = GeneratorLocation + ForwardVector * 94.f;
		break;
	case EGeneratorInteractionPosition::Back:
		TargetLocation = GeneratorLocation - ForwardVector * 110.f;
		break;
	case EGeneratorInteractionPosition::Left:
		TargetLocation = GeneratorLocation - RightVector * 80.f;
		break;
	case EGeneratorInteractionPosition::Right:
		TargetLocation = GeneratorLocation + RightVector * 85.f;
		break;
	default:
		return;
	}
	TargetLocation.Z += 120.0f;  // Z 값 증가

	Player->SetActorLocation(TargetLocation);
} 

void UD1GA_Dracula_DamageGenerator::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	KillerController->SetIgnoreLookInput(false);
	UE_LOG(LogTemp, Log, TEXT("✅ DamageGenerator GAS END "));
}
