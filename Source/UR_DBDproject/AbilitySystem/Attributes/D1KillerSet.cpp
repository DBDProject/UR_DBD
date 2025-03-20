// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/D1KillerSet.h"

UD1KillerSet::UD1KillerSet()
{
	InitWalkSpeed(600.0f);
	InitHellFireChargingWalkSpeed(380.0f);
	InitHellFireAfterShootingWalkSpeed(380.0f);
	InitWolfRunSpeed(480.0f);
	InitBatRunSpeed(650.0f);
	InitHellFireCoolTime(10.0f);
	InitHellFireChargingTime(0.9f);
	InitWolfScentGenTime(5.0f);
	InitWolfScentLivingTime(10.0f);
	InitWolfPounceChargingTime(0.85f);
	InitWolfPounceCoolTime(20.0f);
	InitWolfPounceAttackRange(500.0f);
	InitBreakingTime(2.25f);
	InitFearRange(3200.0f);
}
