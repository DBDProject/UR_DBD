// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/D1ItemBase.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Components/SkeletalMeshComponent.h"

AD1ItemBase::AD1ItemBase()
{
    PrimaryActorTick.bCanEverTick = true;
    MaxUsage = 100.f;
    CurrentUsage = MaxUsage;
    bCanUseItem = true;


    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootScene;

    ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootScene);
}

void AD1ItemBase::BeginPlay()
{
    Super::BeginPlay();
}

void AD1ItemBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// 아이템 사용
void AD1ItemBase::UseItem(AD1SurvivorBase* Survivor)
{
    if (!bCanUseItem || CurrentUsage <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("아이템을 사용할 수 없습니다."));
        return;
    }

    DecreaseUsage(10.f);
}

// 아이템 사용 후 내구도 감소
void AD1ItemBase::DecreaseUsage(float Amount)
{
    if (CurrentUsage > 0.f)
    {
        CurrentUsage -= Amount;
        if (CurrentUsage <= 0.f)
        {
            bCanUseItem = false;
            CurrentUsage = 0.f;
            UE_LOG(LogTemp, Warning, TEXT("아이템 내구도 소진됨!"));
        }
    }
}

// UI에서 게이지로 사용하기 위해 내구도 퍼센트 반환
float AD1ItemBase::GetDurabilityPercentage() const
{
    return (CurrentUsage / MaxUsage) * 100.0f;
}