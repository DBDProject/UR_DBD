// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1ScentSphere.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1ScentSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1ScentSphere();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> Mesh;

	FTimerHandle LifetimeHandle;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	void DestroySelf();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
