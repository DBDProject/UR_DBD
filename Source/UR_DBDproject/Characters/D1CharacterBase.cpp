// Fill out your copyright notice in the Description page of Project Settings.


#include "D1CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "D1PlayerController.h"

// Sets default values
AD1CharacterBase::AD1CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;			// 이동 방향을 자동으로 바라보지 않음
	bUseControllerRotationYaw = false;									// 컨트롤러의 방향을 따라 캐릭터가 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 650.f, 0.f);	// 회전 속도


	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;				// 플레이어가 아니라 컨트롤러 회전을 따름

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false; // 카메라 독립적으로 회전 가능


	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

// Called when the game starts or when spawned
void AD1CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 컨트롤러의 기본 회전값을 설정하여 카메라 방향 조정
	if (Controller)
	{
		Controller->SetControlRotation(FRotator(-45, 0, 0));
	}
}

// Called every frame
void AD1CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AD1CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AD1CharacterBase::SetSprint(bool bSprint)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = bSprint ? RunSpeed : WalkSpeed;
	}
}

