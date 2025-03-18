// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/D1ExitArea.h"
#include "Characters/Survivor/D1SurvivorBase.h"

// Sets default values
AD1ExitArea::AD1ExitArea()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// RootComponent 설정 (SceneComponent 사용)
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 물리 충돌 박스 (Physics Collision)
	PhysicsCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsCollisionBox"));
	PhysicsCollisionBox->SetupAttachment(RootComponent);
	PhysicsCollisionBox->SetBoxExtent(FVector(800.f, 200.f, 500.f)); // 적절한 크기로 설정
	PhysicsCollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	PhysicsCollisionBox->SetGenerateOverlapEvents(false); // 오버랩 판정 비활성화 (물리 전용)


	// 오버랩 감지 박스 (Interaction Box)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 오버랩 감지만 가능하도록 설정
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널 무시
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 플레이어와 오버랩 감지
	InteractionBox->SetBoxExtent(FVector(800.f, 400.f, 500.f));
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화
	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AD1ExitArea::OnOverlapBegin);
}

void AD1ExitArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("생존자가 탈출구에 도착했습니다!"));
}

// Called when the game starts or when spawned
void AD1ExitArea::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AD1ExitArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

