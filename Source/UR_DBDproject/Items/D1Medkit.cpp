// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/D1Medkit.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Items/D1ItemBase.h"

AD1Medkit::AD1Medkit()
{
    ItemName = "구급상자";
    MaxUsage = 100.f;
    CurrentUsage = MaxUsage;
}

void AD1Medkit::UseItem(AD1SurvivorBase* Survivor)
{
    if (!bCanUseItem || !Survivor)
        return;

    UE_LOG(LogTemp, Warning, TEXT("구급상자를 사용하여 치료 속도 증가!"));

    // 치료 속도 증가
    //Survivor->ModifyHealingSpeed(2.0f);  // 기존 속도의 2배

    DecreaseUsage(3.f);
}
