// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/D1ItemBase.h"
#include "D1Toolbox.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1Toolbox : public AD1ItemBase
{
	GENERATED_BODY()

public:
	AD1Toolbox();
	UFUNCTION(NetMulticast, Reliable)
	void UseItem(class AD1SurvivorBase* Survivor);
	UFUNCTION(NetMulticast, Reliable)
	void NotUseItem(class AD1SurvivorBase* Survivor);

};
