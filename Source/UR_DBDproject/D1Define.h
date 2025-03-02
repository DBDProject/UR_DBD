#pragma once

#include "CoreMinimal.h"

#define D(x) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, x); }

UENUM(BlueprintType)
enum class ECreatureState : uint8
{
	None,
	Idle,
	Walk,
	Run,
	Crouch,
	Attack1,
	Skill1,
	InTransform,
	Interactable,
	Dead,
	Parkour,
};

UENUM(BlueprintType)
enum class ESurvivorState : uint8
{
	None,
	Healthy     UMETA(DisplayName = "Healthy"),   // 건강 상태 (기본)
	Injured     UMETA(DisplayName = "Injured"),   // 부상 상태 (살인마 공격 1회)
	Crawl      UMETA(DisplayName = "Crawl")     // 기절 상태 (살인마 공격 2회)
};

UENUM(BlueprintType)
enum class EGeneratorInteractionPosition : uint8
{
	None,
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EVaultType : uint8
{
	Slow UMETA(DisplayName = "Slow"),
	Medium UMETA(DisplayName = "Medium"),
	Fast UMETA(DisplayName = "Fast")
};