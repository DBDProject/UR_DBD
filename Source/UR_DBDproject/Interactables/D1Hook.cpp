// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Hook.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "CineCameraComponent.h"
#include "CineCameraActor.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"


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
    EntityMesh->SetVisibility(false);

    HookReactionCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("HookReaction"));
    HookReactionCamera->SetupAttachment(RootComponent);

    //ExecutionCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("ExecutionCamera"));
    //ExecutionCamera->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UAnimMontage> EntityMontageAsset(TEXT("/Game/Blueprints/Animation/Interactables/AM_Entity.AM_Entity"));
    if (EntityMontageAsset.Succeeded())
    {
        EntityMontage = EntityMontageAsset.Object;
    }
}

void AD1Hook::BeginPlay()
{
    Super::BeginPlay();

    if (EntityMesh)
    {
        DynamicMat_Slot1 = EntityMesh->CreateAndSetMaterialInstanceDynamic(1); // 슬롯 1번
        DynamicMat_Slot0 = EntityMesh->CreateAndSetMaterialInstanceDynamic(0); // 슬롯 0번

        if (DynamicMat_Slot1)
        {
            DynamicMat_Slot1->SetScalarParameterValue(FName("DissolveValue"), 0.0f);
        }
        if (DynamicMat_Slot0)
        {
            DynamicMat_Slot0->SetScalarParameterValue(FName("DissolveValue"), 0.0f);
        }
    }
}

void AD1Hook::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AD1Hook, CurrentDissolveValue);
    DOREPLIFETIME(AD1Hook, bIsSkillCheckEnable);
    DOREPLIFETIME(AD1Hook, bIsSkillCheckFail);
    DOREPLIFETIME(AD1Hook, bEntityVisible);
    DOREPLIFETIME(AD1Hook, bIsHooked);
}

void AD1Hook::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AD1Hook::ActivateEntity()
{
    if (EntityMesh)
    {
        EntityMesh->SetVisibility(true);
        bEntityVisible = true;
    }
}

void AD1Hook::DeactivateEntity()
{
    if (EntityMesh)
    {
        EntityMesh->SetVisibility(false);
        bEntityVisible = false;
        InteractingPlayer = nullptr;
    }
}

void AD1Hook::StartDissolveEffect_Implementation(AD1SurvivorBase* Player)
{
    if (!EntityMesh) return;

    if (!DynamicMat_Slot0)
    {
        DynamicMat_Slot0 = EntityMesh->CreateAndSetMaterialInstanceDynamic(0);
    }
    if (!DynamicMat_Slot1)
    {
        DynamicMat_Slot1 = EntityMesh->CreateAndSetMaterialInstanceDynamic(1);
    }

    if (DynamicMat_Slot0 && DynamicMat_Slot1)
    {
        ActivateEntity();
        GetWorld()->GetTimerManager().SetTimer(DissolveTimer, this, &AD1Hook::UpdateDissolve, 0.05f, true);
        CurrentDissolveValue = 0.0f;
        DissolveStartTime = GetWorld()->GetTimeSeconds();
    }
}

void AD1Hook::UpdateDissolve()
{
    if (!DynamicMat_Slot0 || !DynamicMat_Slot1) return;

    float ElapsedTime = GetWorld()->GetTimeSeconds() - DissolveStartTime;
    CurrentDissolveValue = FMath::Clamp(ElapsedTime / 3.0f, 0.0f, 1.0f);

    if (DynamicMat_Slot0)
        DynamicMat_Slot0->SetScalarParameterValue(FName("DissolveValue"), CurrentDissolveValue);

    if (DynamicMat_Slot1)
        DynamicMat_Slot1->SetScalarParameterValue(FName("DissolveValue"), CurrentDissolveValue);

    if (CurrentDissolveValue >= 1.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(DissolveTimer);
    }
}

void AD1Hook::StartDissolveDisappearEffect_Implementation()
{
    if (!EntityMesh) return;

    if (!DynamicMat_Slot0)
    {
        DynamicMat_Slot0 = EntityMesh->CreateAndSetMaterialInstanceDynamic(0);
    }
    if (!DynamicMat_Slot1)
    {
        DynamicMat_Slot1 = EntityMesh->CreateAndSetMaterialInstanceDynamic(1);
    }

    if (DynamicMat_Slot0 && DynamicMat_Slot1)
    {
        GetWorld()->GetTimerManager().SetTimer(DissolveTimer, this, &AD1Hook::UpdateDissolveDisappear, 0.05f, true);
        DissolveStartTime = GetWorld()->GetTimeSeconds();
    }
}

void AD1Hook::UpdateDissolveDisappear()
{
    if (!DynamicMat_Slot0 || !DynamicMat_Slot1) return;

    float ElapsedTime = GetWorld()->GetTimeSeconds() - DissolveStartTime;
    CurrentDissolveValue = 1.0f - FMath::Clamp(ElapsedTime / 3.0f, 0.0f, 1.0f); // 거꾸로 줄어들도록 설정

    if (DynamicMat_Slot0)
        DynamicMat_Slot0->SetScalarParameterValue(FName("DissolveValue"), CurrentDissolveValue);

    if (DynamicMat_Slot1)
        DynamicMat_Slot1->SetScalarParameterValue(FName("DissolveValue"), CurrentDissolveValue);

    if (CurrentDissolveValue <= 0.0f) // 완전히 사라지면 비활성화
    {
        GetWorld()->GetTimerManager().ClearTimer(DissolveTimer);
        DeactivateEntity();
    }
}

void AD1Hook::PlayEntityMontage(FName Section)
{
    if (!EntityMesh || !EntityMontage) return;
    EntityMesh->GetAnimInstance()->Montage_Play(EntityMontage);
    EntityMesh->GetAnimInstance()->Montage_JumpToSection(Section, EntityMontage);
}

void AD1Hook::StartHookCameraCutscene()
{
    if (!InteractingPlayer) return;

    APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController());

    if (this && PC)
    {
        PC->SetViewTargetWithBlend(this, 0.3f); // 갈고리 고유 카메라로 전환
    }
}

void AD1Hook::EndHookCameraCutscene()
{
    if (!InteractingPlayer) return;

    APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController());

    if (PC)
    {
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            PC->SetViewTargetWithBlend(PlayerPawn, 0.3f); // 플레이어 시점 복귀
        }
    }
}

void AD1Hook::PlayHookExecutionSequence()
{
    if (!HookExecutionSequence && !InteractingPlayer) return;
    if (!(InteractingPlayer->GetController())) return;

    if (InteractingPlayer->GetController()->IsLocalPlayerController())
    {
        FMovieSceneSequencePlaybackSettings Settings;
        Settings.bAutoPlay = true;

        ALevelSequenceActor* HookSequenceActor = nullptr;
        ULevelSequencePlayer* HookSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
            GetWorld(), HookExecutionSequence, Settings, HookSequenceActor
        );

        if (HookSequencePlayer)
        {
            FMovieSceneObjectBindingID BindingID = HookSequenceActor->GetSequence()->FindBindingByTag("CameraTarget");
            HookSequenceActor->SetBinding(BindingID, { ExecutionCameraActor });

            HookSequencePlayer->Play();
        }
    }
}