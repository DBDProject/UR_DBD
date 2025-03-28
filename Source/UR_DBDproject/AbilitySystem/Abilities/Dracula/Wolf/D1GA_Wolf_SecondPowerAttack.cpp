// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Dracula/Wolf/D1GA_Wolf_SecondPowerAttack.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Interactables/D1Pallet.h"

UD1GA_Wolf_SecondPowerAttack::UD1GA_Wolf_SecondPowerAttack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UD1GA_Wolf_SecondPowerAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
	{
		return false;
	}

	return true;
}

void UD1GA_Wolf_SecondPowerAttack::ActivateAbility(
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
		Multicast_SecondWolfAnim(Killer, FName("In"));
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UD1GA_Wolf_SecondPowerAttack::InMontageEnded);
	WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_PowerAttack.Get());

}

void UD1GA_Wolf_SecondPowerAttack::InMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* WolfAnimInstance = Killer->GetMesh()->GetAnimInstance();
	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);
	WolfAnimInstance->Montage_JumpToSection(FName("Swing"), Wolf_PowerAttack.Get());
	if (HasAuthority(&CurrentActivationInfo))
	{
		Multicast_SecondWolfAnim(Killer, FName("Swing"));
	}

	GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &UD1GA_Wolf_SecondPowerAttack::DashToTarget, 0.01f, true);
	GetWorld()->GetTimerManager().SetTimer(DashEndTimer, this, &UD1GA_Wolf_SecondPowerAttack::EndDash, 0.38f, true);

}

void UD1GA_Wolf_SecondPowerAttack::DashToTarget()
{
	UE_LOG(LogTemp, Warning, TEXT("🚨 Second 대쉬중"));
	FVector CurrentLocation = Killer->GetActorLocation();
	FVector TargetForward = Killer->GetActorForwardVector();
	float MoveSpeed = 500.0f / 0.38f; // 거리 / 시간

	FVector MoveAmount = TargetForward * MoveSpeed * 0.01f;

	FHitResult Hit;
	FVector Start = CurrentLocation + FVector(0, 0, 50);
	FVector End = Start - FVector(0, 0, 1000);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		FVector GroundNormal = Hit.ImpactNormal;
		FVector GroundDirection = FVector::VectorPlaneProject(TargetForward, GroundNormal).GetSafeNormal();

		MoveAmount = GroundDirection * MoveSpeed * 0.01f;
	}

	Killer->SetActorLocation(CurrentLocation + MoveAmount, true);

	PerformWolfAttackTrace();
}

void UD1GA_Wolf_SecondPowerAttack::EndDash()
{
	UE_LOG(LogTemp, Warning, TEXT("🚨 Second 대쉬끝"));
	GetWorld()->GetTimerManager().ClearTimer(DashTimer);
	GetWorld()->GetTimerManager().ClearTimer(DashEndTimer);
	UAnimInstance* WolfAnimInstance = Killer->GetMesh()->GetAnimInstance();
	if (bSurvivorHit)
	{
		WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);
		WolfAnimInstance->Montage_JumpToSection(FName("Hit"), Wolf_PowerAttack.Get());
		if (HasAuthority(&CurrentActivationInfo))
		{
			Multicast_SecondWolfAnim(Killer, FName("Hit"));
		}

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UD1GA_Wolf_SecondPowerAttack::FinalMontageEnded);
		WolfAnimInstance->Montage_SetEndDelegate(EndDelegate, Wolf_PowerAttack.Get());
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UD1GA_Wolf_SecondPowerAttack::PerformWolfAttackTrace()
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
					UE_LOG(LogTemp, Warning, TEXT("팔레트 파괴"));

					pallet->OnDestroy();
					bSurvivorHit = true;

					break;
				}
			}
		}
	}
}

void UD1GA_Wolf_SecondPowerAttack::Multicast_SecondWolfAnim_Implementation(AD1KillerBase* Player, FName SectionName)
{
	UAnimInstance* WolfAnimInstance = Player->GetMesh()->GetAnimInstance();

	WolfAnimInstance->Montage_Play(Wolf_PowerAttack.Get(), 1.0f);
	WolfAnimInstance->Montage_JumpToSection(SectionName, Wolf_PowerAttack.Get());
}

void UD1GA_Wolf_SecondPowerAttack::FinalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UD1GA_Wolf_SecondPowerAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	bSurvivorHit = false;
	Killer->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	UE_LOG(LogTemp, Log, TEXT("✅ Second Wolf Power Attack GAS END "));
}