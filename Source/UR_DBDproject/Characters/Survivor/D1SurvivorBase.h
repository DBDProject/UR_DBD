// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1CharacterBase.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"
#include "D1SurvivorBase.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1SurvivorBase : public AD1CharacterBase
{
	GENERATED_BODY()

public:
	AD1SurvivorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	virtual void InitAbilitySystem() override;
	virtual void PossessedBy(AController* NewController) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void SmoothCameraTransition(float DeltaTime);

	// 콜리전 이벤트 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// 오버랩 감지용 박스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 생존자가 감지한 오브젝트 저장
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AActor> DetectedObject;

	// 상호작용 중인 발전기 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<class AD1Generator> CurrentGenerator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UD1SurvivorSet> SurvivorSet;

public:
	AActor* GetDetectedObject() const { return DetectedObject.IsValid() ? DetectedObject.Get() : nullptr; }
	AD1Generator* GetCurrentGenerator() const { return CurrentGenerator.IsValid() ? CurrentGenerator.Get() : nullptr; }

	UD1SurvivorSet* GetSurvivoreSet() const { return SurvivorSet; }
};
