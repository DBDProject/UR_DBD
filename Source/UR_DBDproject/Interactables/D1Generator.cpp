// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Generator.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Animation/D1GeneratorAnim.h"
#include "Characters/Survivor/D1SurvivorController.h"
#include "System/D1GameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AD1Generator::AD1Generator()
{
    PrimaryActorTick.bCanEverTick = true;

    // RootComponent 설정 (SceneComponent 사용)
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // 물리 충돌 박스 (Physics Collision)
    PhysicsCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsCollisionBox"));
    PhysicsCollisionBox->SetupAttachment(RootComponent);
    PhysicsCollisionBox->SetBoxExtent(FVector(50.f, 50.f, 100.f)); // 적절한 크기로 설정
    PhysicsCollisionBox->SetCollisionProfileName(TEXT("BlockAll"));  // 모든 물리 충돌 감지
    PhysicsCollisionBox->SetGenerateOverlapEvents(false); // 오버랩 판정 비활성화 (물리 전용)

    // 오버랩 감지 박스 (Interaction Box)
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetBoxExtent(FVector(100.f, 100.f, 150.f)); // 기존보다 살짝 크게
    InteractionBox->SetCollisionProfileName(TEXT("Trigger")); // 오버랩 전용
    InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

    // 발전기 메쉬
    GeneratorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GeneratorMesh"));
    GeneratorMesh->SetupAttachment(RootComponent);

    // 발전기 몽타주
    static ConstructorHelpers::FObjectFinder<UAnimMontage> GeneratorMontageAsset(TEXT("/Game/Blueprints/Animation/Interactables/AM_Generator_Fail.AM_Generator_Fail"));
    if (GeneratorMontageAsset.Succeeded())
    {
        G_GeneratorMontage = GeneratorMontageAsset.Object;
    }
}

// Called when the game starts or when spawned
void AD1Generator::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            SetOwner(PC);
        }
    }
}

void AD1Generator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AD1Generator, bIsRepairing);
    DOREPLIFETIME(AD1Generator, bIsCompleteRepair);
    DOREPLIFETIME(AD1Generator, bIsRepairBlocked);
    DOREPLIFETIME(AD1Generator, RepairProgress);
    DOREPLIFETIME(AD1Generator, CurrentState);
    DOREPLIFETIME(AD1Generator, InteractionPosition);
}

// Called every frame
void AD1Generator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!HasAuthority()) return; // 서버에서만 실행

    if (bIsCompleteRepair) return;

    // 발전기 수리 진행
    if (bIsRepairing && RepairProgress < 100.f)
    {
        // 수리 속도 계산
        float RepairSpeed = 1.0f;
        int NumPlayers = RepairingPlayers.Num();

        // 수리시 틱 데미지 중지
        GetWorld()->GetTimerManager().ClearTimer(DamageTimer);

        switch (NumPlayers)
        {
        case 1: RepairSpeed = 1.0f; break;
        case 2: RepairSpeed = 1.7f; break;
        case 3: RepairSpeed = 2.1f; break;
        case 4: RepairSpeed = 2.2f; break;
        default: RepairSpeed = 1.0f; break;
        }

        // 진행도 증가
        RepairProgress += RepairSpeed * DeltaTime;

        Multicast_UpdateRepairProgress(RepairProgress);

        UE_LOG(LogTemp, Warning, TEXT("[Server] Players : %d, Repair Progress: %f"), RepairingPlayers.Num(),RepairProgress);
    }

    if (RepairProgress >= 100.0f)
    {
        CompleteRepair();
    }
}

EGeneratorInteractionPosition AD1Generator::FindInteractionPosition(AD1CharacterBase* Survivor)
{
    if (!Survivor) return EGeneratorInteractionPosition::None;

    // 발전기 위치 및 방향 벡터 가져오기
    FVector GeneratorLocation = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector(); // 발전기의 "앞" 방향
    FVector RightVector = GetActorRightVector();     // 발전기의 "오른쪽" 방향

    // 플레이어 위치 가져오기
    FVector PlayerLocation = Survivor->GetActorLocation();

    // 플레이어 → 발전기로 향하는 방향 벡터 계산
    FVector DirectionToPlayer = (PlayerLocation - GeneratorLocation).GetSafeNormal();

    // 내적(Dot Product) 계산
    float ForwardDot = FVector::DotProduct(ForwardVector, DirectionToPlayer);
    float RightDot = FVector::DotProduct(RightVector, DirectionToPlayer);

    // 정면(Front) 또는 후면(Back) 판별
    if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot)) 
    {
        if (ForwardDot > 0) return EGeneratorInteractionPosition::Front;
        return EGeneratorInteractionPosition::Back;
    }

    // 좌우 판별 
    if (RightDot > 0) return EGeneratorInteractionPosition::Right;
    return EGeneratorInteractionPosition::Left;
}
 
void AD1Generator::StartRepair(AD1SurvivorBase* Player, EGeneratorInteractionPosition Position)
{
    if (!Player || bIsRepairBlocked) return;

    if (RepairingPositions.Contains(Position))
    {
        return;
    }

    if (!HasAuthority())
    {
        return;
    }

    if (!RepairingPlayers.Contains(Player))
    {
        RepairingPlayers.Add(Player);
        RepairingPositions.Add(Position, Player);
    }

    // 모든 클라이언트에 수리 상태 동기화
    Multicast_SetRepairState(Player, true, Position);
}

void AD1Generator::StopRepair(AD1SurvivorBase* Player)
{
    if (!HasAuthority())
    {
        return;
    }
    auto Position = Player->GetInteractionPosition();
    RepairingPositions.Remove(Position);
    RepairingPlayers.Remove(Player);

    if (RepairingPlayers.Num() == 0)
    {
        bIsRepairing = false;
        UE_LOG(LogTemp, Warning, TEXT("발전기 수리 중단!"));
    }
}

void AD1Generator::Multicast_SetRepairState_Implementation(AD1SurvivorBase* Player, bool bRepairing, EGeneratorInteractionPosition Position)
{
    if (!Player || bIsRepairBlocked) return;

    // 이동 불가능 설정
    Player->GetCharacterMovement()->DisableMovement();
    Player->GetCharacterMovement()->StopMovementImmediately();
    Player->SetIsRepairing(true);
    bIsRepairing = true;
    InteractionPosition = Position;

    if (AD1SurvivorController* PC = Cast<AD1SurvivorController>(Player->GetController()))
    {
        if (PC->IsLocalPlayerController())
        {
            PC->RepairDelegate_Start();          
        }
    }
}

void AD1Generator::Multicast_UpdateRepairProgress_Implementation(float NewProgress)
{
    RepairProgress = NewProgress;
}

void AD1Generator::OnDamage()
{
	if (DamageCount > 8) {
		UE_LOG(LogTemp, Warning, TEXT("발전기 최대 손상 횟수 초과"));
		return;
	}
    float damage = RepairProgress * 0.05;

    RepairProgress -= damage;
	if (RepairProgress < 0.0f) RepairProgress = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("발전기 손상 %.2f"), damage);
    UE_LOG(LogTemp, Warning, TEXT("발전기 진행도 %.2f"), RepairProgress);

    DamageCount++;

    GetWorld()->GetTimerManager().SetTimer(DamageTimer, this, &AD1Generator::DamagePerSeconds, 1.0f, true);
}

void AD1Generator::DamagePerSeconds()
{
    float TickDamage = RepairProgress * 0.0025;
	RepairProgress -= TickDamage;
    if (RepairProgress < 0.0f) {
        RepairProgress = 0.0f;
        GetWorld()->GetTimerManager().ClearTimer(DamageTimer);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("발전기 진행도 %.2f"), RepairProgress);
}

void AD1Generator::OnSkillCheckSuccess()
{
    if (!HasAuthority())
    {
        if (RepairingPlayers.Num() == 0) return;

        RepairingPlayers[0]->Server_RequestSkillCheckSuccess(this);
        return;
    }
    RepairProgress += 1.0f;
}

void AD1Generator::OnSkillCheckFail(AD1SurvivorBase* Player)
{
    if (!HasAuthority())
    {
        Player->Server_RequestSkillCheckFail(this);
        return;
    }

    Multi_OnSkillCheckFail(Player);
}

void AD1Generator::Multi_OnSkillCheckFail_Implementation(AD1SurvivorBase* Player)
{
    if (!Player) return;

    GeneratorMesh->GetAnimInstance()->Montage_Play(G_GeneratorMontage);
    Player->PlayAnimMontage(Player->S_GeneratorMontage, 1.0f);
    bIsFail = true;
    bIsRepairBlocked = true;
    Player->SetIsFail(true);
    Player->StopRepair();

    RepairProgress -= 5.0f;
    if (RepairProgress < 0.0f) RepairProgress = 0.0f;


    UE_LOG(LogTemp, Warning, TEXT("스킬 체크 실패! 3초간 수리 불가"));

    GetWorldTimerManager().SetTimer(RepairBlockTimer, this, &AD1Generator::EnableRepair, 3.0f, false);
}

void AD1Generator::StopRepairAll()
{
    if (RepairingPlayers.Num() == 0) return;

    bIsRepairBlocked = true;
    bIsRepairing = false;
    SetOwner(nullptr);

    Multicast_StopRepairAll();

    RepairingPlayers.Reset();
}

void AD1Generator::Multicast_StopRepairAll_Implementation()
{
    for (auto Player : RepairingPlayers)
    {
        if (Player)
        {
            Player->SetIsRepairing(false);
            Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Multicast: 모든 플레이어 수리 중단"));
}

void AD1Generator::EnableRepair()
{
    bIsRepairBlocked = false; // 수리 차단 해제
    UE_LOG(LogTemp, Warning, TEXT("수리 가능 상태로 복구됨"));
}

void AD1Generator::CompleteRepair()
{
    RepairProgress = 100.0f;
    bIsCompleteRepair = true;

    StopRepairAll();

    if (HasAuthority())
    {
        BP_CompleteRepair();
    }

    UE_LOG(LogTemp, Warning, TEXT("발전기 수리 완료! 모든 플레이어에게 알림"));
    if (AD1GameState* GameState = GetWorld()->GetGameState<AD1GameState>())
    {
        GameState->UpdateGeneratorState();
    }
}