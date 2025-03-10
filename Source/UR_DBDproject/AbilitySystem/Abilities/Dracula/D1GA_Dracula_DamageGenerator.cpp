// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/D1GA_Dracula_DamageGenerator.h"
#include "Interactables/D1Generator.h"

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
	if (!Killer)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Killer is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

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

	MoveToGeneratorPosition(Pos);
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

void UD1GA_Dracula_DamageGenerator::MoveToGeneratorPosition(EGeneratorInteractionPosition Position)
{
	AD1Generator* Generator = Killer->GetCurrentGenerator();
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

	Killer->SetActorLocation(TargetLocation);

	// 플레이어 방향을 발전기로 조정 (자동 회전)
	FRotator LookAtRotation = (GeneratorLocation - TargetLocation).Rotation();
	LookAtRotation.Pitch = 0.0f;  // 상하 회전을 고정하여 땅을 보지 않도록 설정
	LookAtRotation.Roll = 0.0f;   // 불필요한 기울기 방지
	Killer->SetActorRotation(LookAtRotation);
}

void UD1GA_Dracula_DamageGenerator::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("✅ DamageGenerator GAS END "));
}
