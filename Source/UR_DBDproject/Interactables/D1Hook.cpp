// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Hook.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AD1Hook::AD1Hook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // 오버랩 감지 박스 (Interaction Box)
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetBoxExtent(FVector(100.f, 100.f, 150.f)); // 기존보다 살짝 크게
    InteractionBox->SetCollisionProfileName(TEXT("Trigger")); // 오버랩 전용
    InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	HookMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HookMesh"));
	HookMesh->SetupAttachment(RootComponent);

    EntityMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EntityMesh"));
    EntityMesh->SetupAttachment(RootComponent);
}

void AD1Hook::BeginPlay()
{
    Super::BeginPlay();
    EntityMesh->SetVisibility(false);
}

void AD1Hook::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AD1Hook::UpdateEntityEffect(float HookHealth)
{
    if (!EntityMesh) return;

    // HookHealth 값에 따라 DissolveValue 계산
    float DissolveValue = FMath::Clamp(1.0f - (HookHealth / 100.f), 0.0f, 1.0f);

    // 두 개의 머티리얼 인스턴스에 같은 값 적용
    UMaterialInstanceDynamic* DynamicMat1 = EntityMesh->CreateAndSetMaterialInstanceDynamic(0);
    UMaterialInstanceDynamic* DynamicMat2 = EntityMesh->CreateAndSetMaterialInstanceDynamic(1);

    if (DynamicMat1)
    {
        DynamicMat1->SetScalarParameterValue(FName("DissolveValue"), DissolveValue);
    }

    if (DynamicMat2)
    {
        DynamicMat2->SetScalarParameterValue(FName("DissolveValue"), DissolveValue);
    }
}
