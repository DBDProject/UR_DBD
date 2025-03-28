// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/D1ItemBase.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
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


void AD1ItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AD1ItemBase, CurrentUsage);
}
void AD1ItemBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// 아이템 사용 후 내구도 감소
void AD1ItemBase::DecreaseUsage(float Amount)
{
    if (ItemName == "Medkit")
    {
        if (!(ItemOwner->GetSurvivorState() == ESurvivorState::Injured))
        {
            UE_LOG(LogTemp, Warning, TEXT("아이템 사용 중지!"));
            StopAutoDecreaseUsage();
            ItemOwner->NotUseCurrentItem();
            return;
        }
    }
    else if (ItemName == "Toolbox")
    {
        // TODO
    }

    if (CurrentUsage > 0.f)
    {
        CurrentUsage -= Amount;
        UpdateCurrentUsage(CurrentUsage);
        UE_LOG(LogTemp, Warning, TEXT("[아이템 내구도] 내구도: %.2f%%"), CurrentUsage);
        if (CurrentUsage <= 0.f)
        {
            bCanUseItem = false;
            CurrentUsage = 0.f;
            UE_LOG(LogTemp, Warning, TEXT("아이템 내구도 소진됨!"));
            ItemOwner->NotUseCurrentItem();
        }
    }
}

void AD1ItemBase::UpdateCurrentUsage_Implementation(float Usage)
{
    CurrentUsage = Usage;
}
// UI에서 게이지로 사용하기 위해 내구도 퍼센트 반환
float AD1ItemBase::GetDurabilityPercentage() const
{
    return (CurrentUsage / MaxUsage) * 100.0f;
}

void AD1ItemBase::ActivateItem()
{
    if (ItemMesh)
    {
        ItemMesh->SetVisibility(true);
        IsVisible = true;
    }
}

void AD1ItemBase::DeactivateItem()
{
    if (ItemMesh)
    {
        ItemMesh->SetVisibility(false);
        IsVisible = false;
    }
}

void AD1ItemBase::StartAutoDecreaseUsage(float Amount)
{
    // 타이머 중복 실행 방지
    if (!GetWorldTimerManager().IsTimerActive(UsageDecreaseTimerHandle))
    {
        GetWorldTimerManager().SetTimer(
            UsageDecreaseTimerHandle,
            FTimerDelegate::CreateUObject(this, &AD1ItemBase::DecreaseUsage, Amount),
            0.1f,
            true
        );
    }
}

void AD1ItemBase::StopAutoDecreaseUsage()
{
    GetWorldTimerManager().ClearTimer(UsageDecreaseTimerHandle);
}