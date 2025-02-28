// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerBase.h"
#include "Characters/Killer/D1KillerController.h"
#include "Animation/D1KillerBaseAnim.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Characters/Survivor/D1SurvivorBase.h"
#include "Interactables/D1Generator.h"
#include "Interactables/D1VaultObject.h"
#include "Interactables/D1Pallet.h"

AD1KillerBase::AD1KillerBase()
{
	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Dracula/SKM_Dracula.SKM_Dracula'")
	);
	if (CharacterMeshAsset.Succeeded())
	{
		CharacterMesh->SetSkeletalMesh(CharacterMeshAsset.Object);
		CharacterMesh->SetOwnerNoSee(true);
		CharacterMesh->SetupAttachment(GetCapsuleComponent());
		CharacterMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
		//CharacterMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load Character Mesh asset!"));
	}

	FPVMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPVMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FPVMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Dracula/SKM_Dracula_FPV.SKM_Dracula_FPV'")
	);
	if (FPVMeshAsset.Succeeded())
	{
		FPVMesh->SetSkeletalMesh(FPVMeshAsset.Object);
		FPVMesh->SetOnlyOwnerSee(true);
		FPVMesh->SetupAttachment(CharacterMesh);
		//FPVMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load FPVMesh asset!"));
	}

	WolfMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WolfMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WolfMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Wolf/SKM_Wolf.SKM_Wolf'")
	);
	if (WolfMeshAsset.Succeeded())
	{
		WolfMesh->SetSkeletalMesh(WolfMeshAsset.Object);
		WolfMesh->SetupAttachment(GetCapsuleComponent());
		WolfMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
		WolfMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
		WolfMesh->SetHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load WolfMesh asset!"));
	}

	BatMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BatMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BatMeshAsset(
		TEXT("/Script/Engine.SkeletalMesh'/Game/Art/Characters/Killer/Dracula/Meshes/Bat/SKM_Bat.SKM_Bat'")
	);
	if (BatMeshAsset.Succeeded())
	{
		BatMesh->SetSkeletalMesh(BatMeshAsset.Object);
		BatMesh->SetupAttachment(GetCapsuleComponent());
		BatMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 30.0f), FRotator(0.0f, -90.0f, 0.0f));
		BatMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
		BatMesh->SetHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load BatMesh asset!"));
	}

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 125.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = false;				// 플레이어가 아니라 컨트롤러 회전을 따름

	BatSpringArm = CreateDefaultSubobject<USpringArmComponent>("BatSpringArm");
	BatSpringArm->SetupAttachment(BatMesh);
	BatSpringArm->TargetArmLength = 0.0f;
	BatSpringArm->bUsePawnControlRotation = false;				// 플레이어가 아니라 컨트롤러 회전을 따름

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(SpringArm);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.f, 10.0f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	WolfCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("WolfCamera"));
	WolfCameraComponent->SetupAttachment(WolfMesh);
	WolfCameraComponent->SetRelativeLocationAndRotation(FVector(0.f, -160.f, 140.0f), FRotator(0.0f, 90.0f, 0.0f)); // Position the camera
	WolfCameraComponent->bUsePawnControlRotation = true;
	WolfCameraComponent->Deactivate();

	BatCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("BatCamera"));
	BatCameraComponent->SetupAttachment(BatSpringArm);
	BatCameraComponent->SetRelativeLocationAndRotation(FVector(0.f, -150.f, 0.0f), FRotator(0.0f, 90.0f, 0.0f));  // Position the camera
	BatCameraComponent->bUsePawnControlRotation = true;
	BatCameraComponent->Deactivate();

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false; // 카메라 독립적으로 회전 가능
	Camera->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 90.f), FRotator(-30.0f, 0.0f, 0.0f)); // Position the camera

	// 상호작용 감지용 박스 컴포넌트 (상호작용 범위를 넓게 설정)
	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollider"));
	InteractionBox->SetupAttachment(GetCapsuleComponent());
	InteractionBox->SetBoxExtent(FVector(100.0f, 100.0f, 150.f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
	InteractionBox->SetGenerateOverlapEvents(true); // 오버랩 감지 활성화

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->SetupAttachment(CharacterMesh, TEXT("joint_RingERT_01")); // 오른손에 부착
	AttackCollision->SetBoxExtent(FVector(0.5f, 0.5f, 0.5f));
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackCollision->SetActive(false);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AD1KillerBase::BeginPlay()
{
	Super::BeginPlay();

	// 모든 카메라 비활성화
	if (WolfCameraComponent) WolfCameraComponent->Deactivate();
	if (BatCameraComponent) BatCameraComponent->Deactivate();
	if (FirstPersonCameraComponent) FirstPersonCameraComponent->Activate();

	// 기본 카메라 활성화
	if (Camera)
	{
		Camera->Deactivate();
	}

	if (InteractionBox)
	{
		// 콜리전 이벤트 바인딩
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AD1KillerBase::OnOverlapObjectBegin);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AD1KillerBase::OnOverlapObjectEnd);
	}

	if (AttackCollision)
	{
		AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AD1KillerBase::OnOverlapPlayerBegin);
		AttackCollision->OnComponentEndOverlap.AddDynamic(this, &AD1KillerBase::OnOverlapPlayerEnd);
	}
}

void AD1KillerBase::HandleGameplayEvent(FGameplayTag EventTag)
{
	AD1KillerController* KC = Cast<AD1KillerController>(GetController());
	if (KC)
	{
		KC->HandleGameplayEvent(EventTag);
	}
}

void AD1KillerBase::SwitchCamera(ETransformationState NewState)
{
	if (Camera) Camera->Deactivate();
	if (FirstPersonCameraComponent) FirstPersonCameraComponent->Deactivate();
	if (WolfCameraComponent) WolfCameraComponent->Deactivate();
	if (BatCameraComponent) BatCameraComponent->Deactivate();

	if (NewState == ETransformationState::Dracula)
	{
		if (FirstPersonCameraComponent) FirstPersonCameraComponent->Activate();
		return;
	}
	if (NewState == ETransformationState::Wolf)
	{
		if (WolfCameraComponent) WolfCameraComponent->Activate();
		return;
	}
	if (NewState == ETransformationState::Bat)
	{
		if (BatCameraComponent) BatCameraComponent->Activate();
		return;
	}
}

void AD1KillerBase::OnOverlapObjectBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AD1Generator* Generator = Cast<AD1Generator>(OtherActor))
	{
		DetectedObject = OtherActor;
		CurrentGenerator = Generator;
	}

	if (AD1Pallet* Pallet = Cast<AD1Pallet>(OtherActor))
	{
		DetectedObject = OtherActor;
		CurrentPallet = Pallet;
	}
}

void AD1KillerBase::OnOverlapObjectEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (DetectedObject == OtherActor)
	{
		if (AD1Generator* Generator = Cast<AD1Generator>(DetectedObject.Get()))
		{
			CurrentGenerator = nullptr;
		}

		if (AD1Pallet* Pallet = Cast<AD1Pallet>(DetectedObject.Get()))
		{
			CurrentPallet = nullptr;
		}

		if (OtherActor->ActorHasTag("Vaultable"))
		{
			VaultTarget = nullptr;
		}

		DetectedObject = nullptr;
	}
}

void AD1KillerBase::OnOverlapPlayerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!AttackCollision->IsActive())
		return;

	UD1KillerBaseAnim* KC_TPV = Cast<UD1KillerBaseAnim>(CharacterMesh->GetAnimInstance());
	UD1KillerBaseAnim* KC_FPV = Cast<UD1KillerBaseAnim>(FPVMesh->GetAnimInstance());

	if (OtherActor && OtherActor != this)
	{
		UE_LOG(LogTemp, Log, TEXT("공격 적중: %s"), *OtherActor->GetName());

		// 💡 캐스팅이 실패하는지 로그로 확인
		if (AD1SurvivorBase* Survivor = Cast<AD1SurvivorBase>(OtherActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("Survivor 감지됨: %s"), *Survivor->GetName());
			Survivor->TakeDamageFromKiller();

			DetectedSurvivor = Survivor;
			KC_TPV->SetAttackHit(true);
			KC_FPV->SetAttackHit(true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("캐스팅 실패! OtherActor 클래스: %s"), *OtherActor->GetClass()->GetName());
		}
	}
	else
	{
		KC_TPV->SetAttackHit(false);
		KC_FPV->SetAttackHit(false);
	}
}

void AD1KillerBase::OnOverlapPlayerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Log, TEXT("공격 끝"));
	if (DetectedObject == OtherActor)
	{
		DetectedSurvivor = nullptr;
		DetectedObject = nullptr;
	}

}