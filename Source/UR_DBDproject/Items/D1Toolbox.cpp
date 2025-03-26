// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/D1Toolbox.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Items/D1ItemBase.h"

AD1Toolbox::AD1Toolbox()
{
    ItemName = "Toolbox";
    MaxUsage = 150.f;
    CurrentUsage = MaxUsage;
}

void AD1Toolbox::UseItem_Implementation(AD1SurvivorBase* Survivor)
{
    if (!bCanUseItem || CurrentUsage <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("아이템을 사용할 수 없습니다."));
        return;
    }

    if (!Survivor)  return;

    UE_LOG(LogTemp, Warning, TEXT("공구상자를 사용하여 발전기 수리 속도 증가!"));

    // ✅ 발전기 수리 속도 증가
    //Survivor->ModifyRepairSpeed(1.5f);  // 기존 속도의 1.5배

    DecreaseUsage(3.f);
}

void AD1Toolbox::NotUseItem_Implementation(AD1SurvivorBase* Survivor)
{
}

