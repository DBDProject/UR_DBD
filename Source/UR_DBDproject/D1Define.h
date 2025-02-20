#pragma once

#include "CoreMinimal.h"

#define D(x) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, x); }

UENUM(BlueprintType)
enum class ECreatureState : uint8
{
	None,
	Walk,
	Run,
	Crouch,
	Interactable,
	Skill,
	Dead,
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
