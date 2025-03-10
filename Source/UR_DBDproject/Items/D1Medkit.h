// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/D1ItemBase.h"
#include "D1Medkit.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1Medkit : public AD1ItemBase
{
	GENERATED_BODY()

public:
	AD1Medkit();

	virtual void UseItem(AD1SurvivorBase* Survivor) override;

};
