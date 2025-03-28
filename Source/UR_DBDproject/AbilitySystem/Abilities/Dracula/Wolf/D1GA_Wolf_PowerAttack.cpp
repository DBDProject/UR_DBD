// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/Wolf/D1GA_Wolf_PowerAttack.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Interactables/D1Pallet.h"

UD1GA_Wolf_PowerAttack::UD1GA_Wolf_PowerAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Wolf_PowerAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}

	return true;
}

void UD1GA_Wolf_PowerAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (WolfCooldownEffect)
	{
		FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(WolfCooldownEffect, GetAbilityLevel());
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

	if (!Wolf_PowerAttack)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 Montage is NULL!"));
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

	Killer->GetCharacterMovement()->MaxWalkSpeed = 100.0f;
	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

	WolfAnimInstance->Montage_JumpToSection(FName("In"), Wolf_PowerAttack.Get());
	if (HasAuthority(&ActivationInfo))
	{
		Multicast_WolfAnim(Killer, FName("In"));
	}

	ChargingStartTime = GetWorld()->GetTimeSeconds();

	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
}

void UD1GA_Wolf_PowerAttack::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	float ChargingDuration = GetWorld()->GetTimeSeconds() - ChargingStartTime;

	UAnimInstance* WolfAnimInstance = Killer->GetMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);

	if (ChargingDuration >= WolfChargeDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("🚨 차징 성공"));
		WolfAnimInstance->Montage_JumpToSection(FName("Swing"), Wolf_PowerAttack.Get());
		if (HasAuthority(&ActivationInfo))
		{
			Multicast_WolfAnim(Killer, FName("Swing"));
		}

		FVector StartLocation = Killer->GetActorLocation();
		FVector Forward = Killer->GetActorForwardVector();
		TargetLocation = StartLocation + Forward * 500.0f;

		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &UD1GA_Wolf_PowerAttack::DashToTarget, 0.01f, true);

		KillerController->comboIndex = 1;
	}
	else
	{
		KillerController->SetCanSecondPounce(false);
		FinalMontageEnded(Wolf_PowerAttack.Get(), false);
	}
}

void UD1GA_Wolf_PowerAttack::DashToTarget()
{
	UE_LOG(LogTemp, Warning, TEXT("🚨 대쉬중"));
	FVector CurrentLocation = Killer->GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	float MoveSpeed = 500.0f / 0.38f; // 거리 / 시간

	FVector MoveAmount = Direction * MoveSpeed * 0.01f;
	Killer->SetActorLocation(CurrentLocation + MoveAmount);

	PerformWolfAttackTrace();

	if ((Killer->GetActorLocation() - TargetLocation).Size() < 1.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("🚨 대쉬끝"));
		GetWorld()->GetTimerManager().ClearTimer(DashTimer);
		Killer->SetActorLocation(TargetLocation);

		EndDash();
	}
}

void UD1GA_Wolf_PowerAttack::EndDash()
{
	UAnimInstance* WolfAnimInstance = Killer->GetMesh()->GetAnimInstance();
	if (bSurvivorHit)
	{
		WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);
		WolfAnimInstance->Montage_JumpToSection(FName("Hit"), Wolf_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_WolfAnim(Killer, FName("Hit"));
		}

		KillerController->SetCanSecondPounce(false);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UD1GA_Wolf_PowerAttack::FinalMontageEnded);
		WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_PowerAttack.Get());
		return;
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UD1GA_Wolf_PowerAttack::PerformWolfAttackTrace()
{
	FVector TraceStart = Killer->GetMesh()->GetSocketLocation("nose");
	FVector TraceEnd = TraceStart + (Killer->GetActorForwardVector() * 100.f);

	float TraceRadius = 50.0f;

	// 트레이스 결과값 저장용
	TArray<FHitResult> HitResults;
	ECollisionChannel TraceChannel = ECC_Pawn;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Killer);


	// SphereTraceMulti
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	// For Debugging
	//float DebugDrawTime = 2.f;
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, DebugDrawTime);
	//DrawDebugSphere(GetWorld(), TraceEnd, TraceRadius, 12, FColor::Red, false, DebugDrawTime);

	if (bHit && !bSurvivorHit)
	{
		for (auto& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != Killer)
			{
				UE_LOG(LogTemp, Warning, TEXT("공격 적중! : %s"), *HitActor->GetName());
				if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(HitActor))
				{
					UE_LOG(LogTemp, Warning, TEXT("서바이버에게 피해 적용: %s"), *Survivor->GetName());

					Survivor->TakeDamageFromKiller();
					bSurvivorHit = true;

					break;
				}
				else if (AD1Pallet* pallet = Cast<AD1Pallet>(HitActor))
				{
					if (pallet->GetCurrentState() == EPalletState::Down)
					{
						UE_LOG(LogTemp, Warning, TEXT("팔레트 파괴"));

						pallet->OnDestroy();
						bSurvivorHit = true;
					}
					break;
				}
			}
		}
	}
}

void UD1GA_Wolf_PowerAttack::Multicast_WolfAnim_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* WolfAnimInstance = Player->GetMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);
	WolfAnimInstance->Montage_JumpToSection(SectionName, Wolf_PowerAttack.Get());
}

void UD1GA_Wolf_PowerAttack::FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Wolf_PowerAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	bSurvivorHit = false;
	Killer->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	UE_LOG(LogTemp, Log, TEXT("✅ First Wolf Power Attack GAS END "));
}