// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1Generator.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Animation/D1GeneratorAnim.h"

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

    // 기본값 설정
    InteractingPlayer = nullptr;
    bIsRepaired = false;
    RepairProgress = 0.f;
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
    if (InteractingPlayer.IsValid() && RepairProgress < 100.f)
    {
        float RepairSpeed = InteractingPlayer->GetSurvivoreSet()->GetRepairSpeed();
        RepairProgress = FMath::Clamp(RepairProgress + RepairSpeed * DeltaTime, 0.f, 100.f);

        if (RepairProgress >= 100.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("발전기 수리 완료!"));
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
    if (InteractingPlayer.IsValid()) return;

    InteractingPlayer = Player;

    if (CachedAnimInstance.IsValid())
    {
        CachedAnimInstance.Get()->SetInteractionPosition(Position);
    }

    UE_LOG(LogTemp, Warning, TEXT("발전기 수리 시작!"));
}

void AD1Generator::StopRepair()
{
    if (InteractingPlayer.IsValid())
    {
        InteractingPlayer = nullptr;
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
        InteractingPlayer = nullptr;
        bIsRepaired = false;
        UE_LOG(LogTemp, Warning, TEXT("발전기에서 생존자가 멀어짐!"));
    }
}
