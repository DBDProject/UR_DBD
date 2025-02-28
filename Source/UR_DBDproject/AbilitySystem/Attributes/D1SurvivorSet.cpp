// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/D1SurvivorSet.h"

UD1SurvivorSet::UD1SurvivorSet()
{
	// Healthy
	InitWalkSpeed(250.f);
	InitRunSpeed(500.f);
	InitCrouchSpeed(150.f);

	// Injured
	InitInjWalkSpeed(200.f);
	InitInjRunSpeed(400.f);
	InitInjCrouchSpeed(100.f);

	// Crawl
	InitCrawlSpeed(50.f);
}
