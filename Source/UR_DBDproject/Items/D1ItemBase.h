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

    // 아이템 사용 후 내구도 감소
    UFUNCTION(BlueprintCallable, Category = "Item")
    void DecreaseUsage(float Amount);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
    // 아이템 사용 함수 (각 아이템마다 다르게 구현됨)
    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void UseItem(class AD1SurvivorBase* Survivor);

    // UI에서 내구도를 퍼센트(%)로 표시하기 위한 함수
    UFUNCTION(BlueprintPure, Category = "Item")
    float GetDurabilityPercentage() const;

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

    // 최대 사용 가능 횟수 (ex: 공구상자는 32번 사용 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float MaxUsage;

    // 현재 남은 사용 횟수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
    float CurrentUsage;

    // 아이템 사용 가능 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
    bool bCanUseItem;

};
