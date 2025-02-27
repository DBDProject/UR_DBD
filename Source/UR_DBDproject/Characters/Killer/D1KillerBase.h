// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/D1CharacterBase.h"
#include "D1KillerBase.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
UENUM(BlueprintType)
enum class ETransformationState : uint8
{
	Dracula UMETA(DisplayName = "Dracula"),
	Wolf UMETA(DisplayName = "Wolf"),
	Bat UMETA(DisplayName = "Bat"),
	TransformMode UMETA(DisplayName = "Transform Mode")  // 변신 선택 모드
};
/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API AD1KillerBase : public AD1CharacterBase
{
	GENERATED_BODY()

public:
	AD1KillerBase();

	virtual void HandleGameplayEvent(FGameplayTag EventTag) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	// 오버랩 감지용 박스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> InteractionBox;

	// 생존자가 감지한 오브젝트 저장
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AActor> DetectedObject;
	

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TWeakObjectPtr<class AD1SurvivorBase> DetectedSurvivor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> WolfCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> BatCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> BatSpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> CharacterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> FPVMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> WolfMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KDH)
	TObjectPtr<USkeletalMeshComponent> BatMesh;

private:
	UFUNCTION()
	void OnOverlapObjectBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapObjectEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnOverlapPlayerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapPlayerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	UFUNCTION(BlueprintCallable, Category = KDH_Camera)
	void SwitchCamera(ETransformationState NewState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	UBoxComponent* AttackCollision;

	AActor* GetDetectedObject() const { return DetectedObject.IsValid() ? DetectedObject.Get() : nullptr; }

	TObjectPtr<USkeletalMeshComponent> GetCharacterMesh() const { return CharacterMesh; }
	TObjectPtr<USkeletalMeshComponent> GetFPVMesh() const { return FPVMesh; }
	TObjectPtr<USkeletalMeshComponent> GetWolfMesh() const { return WolfMesh; }
	TObjectPtr<USkeletalMeshComponent> GetBatMesh() const { return BatMesh; }

	TObjectPtr<UCameraComponent> GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	TObjectPtr<UCameraComponent> GetWolfCameraComponent() const { return WolfCameraComponent; }
	TObjectPtr<UCameraComponent> GetBatCameraComponent() const { return BatCameraComponent; }

public:
	ETransformationState nowState;
};
