// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Generator.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Animation/D1GeneratorAnim.h"
#include "Characters/Survivor/D1SurvivorController.h"

AD1Generator::AD1Generator()
{
    PrimaryActorTick.bCanEverTick = true;

    // RootComponent 설정 (SceneComponent 사용)
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
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
}

// Called when the game starts or when spawned
void AD1Generator::BeginPlay()
{
	Super::BeginPlay();
	
    if (InteractionBox)
    {
        // 콜리전 이벤트 바인딩
        InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AD1Generator::OnOverlapBegin);
        InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AD1Generator::OnOverlapEnd);
    }

    CachedAnimInstance = Cast<UD1GeneratorAnim>(GeneratorMesh->GetAnimInstance());
}

// Called every frame
void AD1Generator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // 발전기 수리 진행
    if (bIsRepairing && RepairProgress < 100.f)
    {
        // 수리 속도 계산
        float RepairSpeed = 1.0f;
        int NumPlayers = RepairingPlayers.Num();

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
        UE_LOG(LogTemp, Warning, TEXT("Repair Progress: %f"), RepairProgress);

        if (RepairProgress >= 100.0f)
        {
            CompleteRepair();
        }
    }
}

EGeneratorInteractionPosition AD1Generator::FindInteractionPosition(AD1SurvivorBase* Survivor)
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
    if (bIsRepairBlocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("수리가 현재 차단된 상태입니다!"));
        return;
    }

    if (!RepairingPlayers.Contains(Player))
    {
        RepairingPlayers.Add(Player);
    }
    bIsRepairing = true;

    if (CachedAnimInstance.IsValid())
    {
        CachedAnimInstance.Get()->SetInteractionPosition(Position);
        CachedAnimInstance.Get()->SetIsRepairing(true);
    }

    UE_LOG(LogTemp, Warning, TEXT("발전기 수리 시작!"));
}

void AD1Generator::StopRepair(AD1SurvivorBase* Player)
{
    RepairingPlayers.Remove(Player);

    if (RepairingPlayers.Num() == 0)
    {
        bIsRepairing = false;

        if (CachedAnimInstance.IsValid())
        {
            CachedAnimInstance.Get()->SetIsRepairing(false);
        }

        UE_LOG(LogTemp, Warning, TEXT("발전기 수리 중단!"));
    }
}

void AD1Generator::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("발전기 근처에 생존자가 접근함!"));
    }
}

void AD1Generator::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("발전기에서 생존자가 멀어짐!"));
    }
}

void AD1Generator::OnSkillCheckSuccess()
{
    RepairProgress += 1.0f;
    UE_LOG(LogTemp, Warning, TEXT("스킬 체크 대성공!"));
}

void AD1Generator::OnSkillCheckFail()
{
    RepairProgress -= 10.0f;
    if (RepairProgress < 0.0f) RepairProgress = 0.0f;

    bIsRepairBlocked = false;
    StopRepairAll();

    UE_LOG(LogTemp, Error, TEXT("스킬 체크 실패! 3초간 수리 불가"));

    GetWorldTimerManager().SetTimer(RepairBlockTimer, this, &AD1Generator::EnableRepair, 3.0f, false);
}

void AD1Generator::StopRepairAll()
{
    for (auto& Player : RepairingPlayers)
    {
        if (Player)
        {
            Cast<AD1SurvivorController>(Player->GetController())->StopRepair(); // 각 플레이어의 수리 중단 함수 호출
        }
    }

    // 수리 중인 플레이어 목록 초기화
    RepairingPlayers.Empty();
    bIsRepairing = false;
}

void AD1Generator::EnableRepair()
{
    bIsRepairBlocked = false; // 수리 차단 해제
    UE_LOG(LogTemp, Warning, TEXT("수리 가능 상태로 복구됨"));
}

void AD1Generator::CompleteRepair()
{
    RepairProgress = 100.0f;
    bIsRepairing = false;

    UE_LOG(LogTemp, Warning, TEXT("발전기 수리 완료! 모든 플레이어에게 알림"));

    // 발전기 완료 이벤트 실행 (UI, 사운드, 이펙트 등)
}
