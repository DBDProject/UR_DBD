// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1ItemBase.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1ItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AD1ItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void Tick(float DeltaTime) override;

    // 아이템 사용 후 내구도 감소
    UFUNCTION(BlueprintCallable, Category = "Item")
    void DecreaseUsage(float Amount);

    UFUNCTION(NetMulticast, Reliable)
    void UpdateCurrentUsage(float Usage);
    // Called every frame

public:
    // UI에서 내구도를 퍼센트(%)로 표시하기 위한 함수
    UFUNCTION(BlueprintPure, Category = "Item")
    float GetDurabilityPercentage() const;

    void ActivateItem();
    void DeactivateItem();

    void StartAutoDecreaseUsage(float Amount);
    void StopAutoDecreaseUsage();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    TWeakObjectPtr<class AD1SurvivorBase> ItemOwner;

protected:
    // 루트컴포넌트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> RootScene;

    // 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    TObjectPtr<class USkeletalMeshComponent> ItemMesh;

    // 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString ItemName;

    // 최대 사용 가능 게이지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float MaxUsage;

public:
    // 현재 남은 사용 게이지
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item")
    float CurrentUsage;

    // 아이템 사용 가능 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
    bool bCanUseItem;

    bool IsVisible = true;

    FTimerHandle UsageDecreaseTimerHandle;

public:
    bool GetIsVisible() { return IsVisible; }
};
