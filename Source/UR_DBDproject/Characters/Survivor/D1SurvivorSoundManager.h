// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1SoundManager.h"
#include "D1SurvivorSoundManager.generated.h"

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorSoundManager : public AD1SoundManager
{
	GENERATED_BODY()
	
public:
	virtual void PlayBGM(USoundBase* Music, float FadeTime = 1.0f) override;
};
