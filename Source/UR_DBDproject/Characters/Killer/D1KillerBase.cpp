// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"

AD1KillerBase::AD1KillerBase()
{
	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshAsset(
		TEXT("/Game/Art/Characters/Killer/Dracula/Meshes/SKM_Dracula.SKM_Dracula")
	);
	if (CharacterMeshAsset.Succeeded())
	{
		CharacterMesh->SetSkeletalMesh(CharacterMeshAsset.Object);
		CharacterMesh->SetOwnerNoSee(true);
		CharacterMesh->SetupAttachment(GetCapsuleComponent());
		CharacterMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load Character Mesh asset!"));
	}

	FPVMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPVMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FPVMeshAsset(
		TEXT("/Game/Art/Characters/Killer/Dracula/Meshes/SKM_Dracula_FPV.SKM_Dracula_FPV")
	);
	if (FPVMeshAsset.Succeeded())
	{
		FPVMesh->SetSkeletalMesh(FPVMeshAsset.Object);
		FPVMesh->SetOnlyOwnerSee(true);
		FPVMesh->SetupAttachment(CharacterMesh);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load FPVMesh asset!"));
	}

	WolfMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WolfMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WolfMeshAsset(
		TEXT("/Game/Art/Characters/Killer/Dracula/Wolf/wolf_mesh.wolf_mesh")
	);
	if (WolfMeshAsset.Succeeded())
	{
		WolfMesh->SetSkeletalMesh(WolfMeshAsset.Object);
		WolfMesh->SetupAttachment(GetCapsuleComponent());
		WolfMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
		WolfMesh->SetHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load WolfMesh asset!"));
	}

	BatMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BatMesh"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BatMeshAsset(
		TEXT("/Game/Art/Characters/Killer/Dracula/Bat/bat_mesh.bat_mesh")
	);
	if (BatMeshAsset.Succeeded())
	{
		BatMesh->SetSkeletalMesh(BatMeshAsset.Object);
		BatMesh->SetupAttachment(GetCapsuleComponent());
		BatMesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 88.0f), FRotator(0.0f, -90.0f, 0.0f));
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
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-50.f, 0.f, 0.0f)); // Position the camera
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
	Camera->Deactivate();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AD1KillerBase::BeginPlay()
{
	Super::BeginPlay();

}

void AD1KillerBase::SwitchCamera(ECreatureState NewState)
{
	if (FirstPersonCameraComponent) FirstPersonCameraComponent->Deactivate();
	if (WolfCameraComponent) WolfCameraComponent->Deactivate();
	if (BatCameraComponent) BatCameraComponent->Deactivate();

	if (NewState == ECreatureState::Dracula)
	{
		if (FirstPersonCameraComponent) FirstPersonCameraComponent->Activate();
		return;
	}
	if (NewState == ECreatureState::Wolf)
	{
		if (WolfCameraComponent) WolfCameraComponent->Activate();
		return;
	}
	if (NewState == ECreatureState::Bat)
	{
		if (BatCameraComponent) BatCameraComponent->Activate();
		return;
	}
}