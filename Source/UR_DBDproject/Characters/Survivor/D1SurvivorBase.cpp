// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Survivor/D1SurvivorBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "D1SurvivorState.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1SurvivorSet.h"
#include "Interactables/D1Generator.h"
#include "Components/BoxComponent.h"

AD1SurvivorBase::AD1SurvivorBase()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;			// 이동 방향을 자동으로 바라보지 않음
	bUseControllerRotationYaw = false;									// 컨트롤러의 방향을 따라 캐릭터가 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 650.f, 0.f);	// 회전 속도


	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 150.f;
	SpringArm->bUsePawnControlRotation = true;				// 플레이어가 아니라 컨트롤러 회전을 따름

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false; // 카메라 독립적으로 회전 가능

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	// 상호작용 감지용 박스 컴포넌트 (상호작용 범위를 넓게 설정)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollider"));
	InteractionBox->SetupAttachment(GetCapsuleComponent());
	InteractionBox->SetBoxExtent(FVector(50.f, 50.f, 100.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화
}

void AD1SurvivorBase::BeginPlay()
{
	Super::BeginPlay();

	// 컨트롤러의 기본 회전값을 설정하여 카메라 방향 조정
	if (Controller)
	{
		Controller->SetControlRotation(FRotator(-45, 0, 0));
	}

	if (InteractionBox)
	{
		// 콜리전 이벤트 바인딩
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AD1SurvivorBase::OnOverlapBegin);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AD1SurvivorBase::OnOverlapEnd);
	}
}

void AD1SurvivorBase::InitAbilitySystem()
{
	if (AD1SurvivorState* PS = GetPlayerState<AD1SurvivorState>())
	{
		AbilitySystemComponent = Cast<UD1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		if (!AbilitySystemComponent) return;

		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		AttributeSet = PS->GetD1SurvivorSet();
		SurvivorSet = Cast<UD1SurvivorSet>(AttributeSet);
		if (SurvivorSet)
		{
			GetCharacterMovement()->MaxWalkSpeed = SurvivorSet->GetWalkSpeed();
			GetCharacterMovement()->MaxWalkSpeedCrouched = SurvivorSet->GetCrouchSpeed();
		}
	}
}

void AD1SurvivorBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystem();
}

void AD1SurvivorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SmoothCameraTransition(DeltaTime);
}

// 웅크릴 때 카메라 보간
void AD1SurvivorBase::SmoothCameraTransition(float DeltaTime)
{
	if (!SpringArm) return;

	float DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float CrouchCapsuleHalfHeight = GetCharacterMovement()->CrouchedHalfHeight;

	// 카메라 보간 속도
	float CameraLerpSpeed = 6.f;

	float TargetHeight = bIsCrouched ? CrouchCapsuleHalfHeight * 0.7f : DefaultCapsuleHalfHeight * 0.7f;
	float CurrentHeight = SpringArm->SocketOffset.Z;

	float NewHeight = FMath::FInterpTo(CurrentHeight, TargetHeight, DeltaTime, CameraLerpSpeed);
	SpringArm->SocketOffset = FVector(0.f, 0.f, NewHeight);
}

void AD1SurvivorBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AD1Generator* Generator = Cast<AD1Generator>(OtherActor))
	{
		DetectedObject = Generator;
		CurrentGenerator = Generator;
	}
}

void AD1SurvivorBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (DetectedObject == OtherActor)
	{
		if (AD1Generator* Generator = Cast<AD1Generator>(DetectedObject.Get()))
		{
			Generator->StopRepair(this);
			CurrentGenerator = nullptr;
		}
		DetectedObject = nullptr;
	}
}
