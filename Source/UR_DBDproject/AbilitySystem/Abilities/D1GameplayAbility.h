// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Killer/D1KillerController.h"
#include "Animation/D1KillerBaseAnim.h"
#include "D1Define.h"
#include "Net/UnrealNetwork.h"
#include "D1GameplayAbility.generated.h"

class AABTA_Trace;
/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	AD1KillerBase* Killer = nullptr;
	AD1KillerController* KillerController = nullptr;
};
