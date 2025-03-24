// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1SoundManager.generated.h"

class UAudioComponent;
class USoundBase;
UCLASS()
class UR_DBDPROJECT_API AD1SoundManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1SoundManager();

	UFUNCTION(BlueprintCallable)
	virtual void PlayBGM(USoundBase* Music, float FadeTime = 1.0f);

	UFUNCTION(BlueprintCallable)
	void StopBGM(float FadeTime = 1.0f);

	UFUNCTION(BlueprintCallable)
	virtual void PlaySFX(USoundBase* Sound);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnFadeOutComplete(USoundBase* NewMusic, float FadeTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> BGMComponentA;

	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> BGMComponentB;

	UPROPERTY(VisibleAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> SFXComponent;

	UPROPERTY()
	TObjectPtr<USoundBase> CurrentBGM;
	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentComponent;     // 현재 재생 중
	UPROPERTY()
	TObjectPtr<UAudioComponent> PreviousComponent;    // 페이드아웃 대상

	bool bIsFadingOut;

	FTimerHandle FadeOutTimerHandle;
};
