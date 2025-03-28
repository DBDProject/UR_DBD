// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/D1Medkit.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Items/D1ItemBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AD1Medkit::AD1Medkit()
{
    ItemName = "Medkit";
    MaxUsage = 50.f;
    CurrentUsage = MaxUsage;
}

// 멀티케스트 RPC
void AD1Medkit::UseItem_Implementation(AD1SurvivorBase* Survivor)
{
    if (Survivor->GetController()->IsLocalPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("[현재 아이템] 내구도: %.2f%%"), CurrentUsage);
    }
    if (!bCanUseItem || CurrentUsage <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("아이템을 사용할 수 없습니다."));
        return;
    }

    if (!Survivor)  return;

    if (Survivor->GetSurvivorState() == ESurvivorState::Injured)
    {
        Survivor->GetCharacterMovement()->DisableMovement();
        Survivor->SetIsUsingMedkit(true);
        if (HasAuthority())
        {
            StartAutoDecreaseUsage(1.0f);
        }
    }
}

void AD1Medkit::NotUseItem_Implementation(AD1SurvivorBase* Survivor)
{
    if (!Survivor)  return;

    Survivor->SetIsUsingMedkit(false);
    Survivor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    if (HasAuthority())
    {
        StopAutoDecreaseUsage();
    }
}
