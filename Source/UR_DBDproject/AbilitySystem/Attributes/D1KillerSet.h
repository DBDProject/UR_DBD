// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/D1AttributeSet.h"
#include "D1KillerSet.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1KillerSet : public UD1AttributeSet
{
	GENERATED_BODY()

public:
	UD1KillerSet();

public:
	ATTRIBUTE_ACCESSORS(ThisClass, HellFireChargingWalkSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, HellFireAfterShootingWalkSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, WolfRunSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, BatRunSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, HellFireCoolTime);
	ATTRIBUTE_ACCESSORS(ThisClass, HellFireChargingTime);
	ATTRIBUTE_ACCESSORS(ThisClass, WolfScentGenTime);
	ATTRIBUTE_ACCESSORS(ThisClass, WolfScentLivingTime);
	ATTRIBUTE_ACCESSORS(ThisClass, WolfPounceChargingTime);
	ATTRIBUTE_ACCESSORS(ThisClass, WolfPounceCoolTime);
	ATTRIBUTE_ACCESSORS(ThisClass, WolfPounceAttackRange);
	ATTRIBUTE_ACCESSORS(ThisClass, BreakingTime);
	ATTRIBUTE_ACCESSORS(ThisClass, FearRange);

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData HellFireChargingWalkSpeed; // 지옥불 충전시 이속

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData HellFireAfterShootingWalkSpeed; // 지옥불 발사 후 이속

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData WolfRunSpeed; // 늑대 향기 구체 먹을시 이속

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData BatRunSpeed; // 박쥐 이속

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData HellFireCoolTime; // 지옥불 쿨타임
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData HellFireChargingTime; // 지옥불 차징 시간

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData WolfScentGenTime; // 늑대 구체 젠 타임

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData WolfScentLivingTime; // 늑대 구체 생명 시간

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData WolfPounceChargingTime; // 늑대 덮치기 차징 시간

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData WolfPounceCoolTime; // 늑대 덮치기 쿨타임

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData WolfPounceAttackRange; // 늑대 덮치기 공격 범위
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData BreakingTime; // 파괴하는데 걸리는 시간

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData FearRange; // 공포 범위


};
