// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Killer/D1KillerController.h"
#include "Data/D1InputData.h"
#include "System/D1AssetManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "D1GameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/Killer/D1KillerBase.h"
#include "AbilitySystem/Attributes/D1KillerSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"

AD1KillerController::AD1KillerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AD1KillerController::BeginPlay()
{
	Super::BeginPlay();

	if (const UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("KillerInputData"))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputData->InputMappingContext, 0);
		}
	}

	D1Killer = Cast<AD1KillerBase>(GetCharacter());
	SetCreatureState(ECreatureState::Dracula);
}


void AD1KillerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UD1InputData* InputData = UD1AssetManager::GetAssetByName<UD1InputData>("KillerInputData"))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		auto MoveAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);

		auto LookAction = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);

		auto Attack1Action = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Attack1);
		EnhancedInputComponent->BindAction(Attack1Action, ETriggerEvent::Triggered, this, &ThisClass::Input_Attack1);

		auto Skill1Action = InputData->FindInputActionByTag(D1GameplayTags::Input_Action_Skill1);
		EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Triggered, this, &ThisClass::Input_Skill1);

	}
}

void AD1KillerController::Input_Move(const FInputActionValue& InputValue)
{
	if (!D1Killer) return;

	FVector2D MovementVector = InputValue.Get<FVector2D>();

	KillerSet = Cast<UD1KillerSet>(D1Killer->GetAttributeSet());

	if (MovementVector.X != 0)
	{
		FRotator Rotator = GetControlRotation();
		FVector Direction = UKismetMathLibrary::GetForwardVector(FRotator(0, Rotator.Yaw, 0));
		GetPawn()->AddMovementInput(Direction, MovementVector.X);
	}

	if (MovementVector.Y != 0)
	{
		FRotator Rotator = GetControlRotation();
		FVector Direction = UKismetMathLibrary::GetRightVector(FRotator(0, Rotator.Yaw, 0));
		GetPawn()->AddMovementInput(Direction, MovementVector.Y);
	}
}

void AD1KillerController::Input_Look(const FInputActionValue& InputValue)
{
	if (!D1Killer) return;

	FVector2D LookAxisVector = InputValue.Get<FVector2D>();

	// add yaw and pitch input to controller
	GetPawn()->AddControllerYawInput(LookAxisVector.X);
	GetPawn()->AddControllerPitchInput(-LookAxisVector.Y);
}

void AD1KillerController::Input_Attack1(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

}

void AD1KillerController::Input_Skill1(const FInputActionValue& InputValue)
{
	if (!D1Killer)
		return;

	ECreatureState CurrentState = GetCreatureState();

	if (CurrentState == ECreatureState::Dracula)
	{
		UAnimInstance* DracAnimInst = D1Killer->GetCharacterMesh()->GetAnimInstance();
		UAnimInstance* DracFPVAnimInst = D1Killer->GetFPVMesh()->GetAnimInstance();

		if (DracAnimInst && Dracula_Out_Wolf_Montage)
		{
			float Duration = DracAnimInst->Montage_Play(Dracula_Out_Wolf_Montage, 1.0f);
			DracFPVAnimInst->Montage_Play(Dracula_Out_Wolf_Montage, 1.0f);
			if (Duration > 0.f)
			{
				// 몽타주 종료 델리게이트 설정
				FOnMontageEnded EndDelegate;
				EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
					{
						// 첫 번째 몽타주가 끝나면 Dracula 메시를 숨기고 Wolf 메시를 보이도록 전환
						D1Killer->GetCharacterMesh()->SetHiddenInGame(true);
						D1Killer->GetFPVMesh()->SetHiddenInGame(true);
						D1Killer->GetWolfMesh()->SetHiddenInGame(false);

						D1Killer->SwitchCamera(ECreatureState::Wolf);

						// 두 번째 몽타주(Wolf_In_Dracula)를 Wolf 메시에서 재생
						UAnimInstance* WolfAnimInst = D1Killer->GetWolfMesh()->GetAnimInstance();
						if (WolfAnimInst && Wolf_In_Dracula_Montage)
						{
							WolfAnimInst->Montage_Play(Wolf_In_Dracula_Montage, 1.0f);
						}
						// 상태 전환 처리 (필요한 경우 CreatureState 업데이트)
						SetCreatureState(ECreatureState::Wolf);
					});
				DracAnimInst->Montage_SetEndDelegate(EndDelegate, Dracula_Out_Wolf_Montage);
				return;
			}
		}
	}

	if (CurrentState == ECreatureState::Wolf)
	{
		UAnimInstance* WolfAnimInst = D1Killer->GetWolfMesh()->GetAnimInstance();

		if (WolfAnimInst && Wolf_Out_Bat_Montage)
		{
			float Duration = WolfAnimInst->Montage_Play(Wolf_Out_Bat_Montage, 1.0f);
			if (Duration > 0.f)
			{
				// 몽타주 종료 델리게이트 설정
				FOnMontageEnded EndDelegate;
				EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
					{
						// 첫 번째 몽타주가 끝나면 Dracula 메시를 숨기고 Wolf 메시를 보이도록 전환
						D1Killer->GetWolfMesh()->SetHiddenInGame(true);
						D1Killer->GetBatMesh()->SetHiddenInGame(false);

						D1Killer->SwitchCamera(ECreatureState::Bat);

						// 두 번째 몽타주(Wolf_In_Dracula)를 Wolf 메시에서 재생
						UAnimInstance* BatAnimInst = D1Killer->GetBatMesh()->GetAnimInstance();
						if (BatAnimInst && Bat_In_Wolf_Montage)
						{
							BatAnimInst->Montage_Play(Bat_In_Wolf_Montage, 1.0f);
						}
						// 상태 전환 처리 (필요한 경우 CreatureState 업데이트)
						SetCreatureState(ECreatureState::Bat);
					});
				WolfAnimInst->Montage_SetEndDelegate(EndDelegate, Wolf_Out_Bat_Montage);
				return;
			}
		}
	}

	if (CurrentState == ECreatureState::Bat)
	{
		UAnimInstance* BatAnimInst = D1Killer->GetBatMesh()->GetAnimInstance();

		if (BatAnimInst && Bat_Out_Dracula_Montage)
		{
			float Duration = BatAnimInst->Montage_Play(Bat_Out_Dracula_Montage, 1.0f);
			if (Duration > 0.f)
			{
				// 몽타주 종료 델리게이트 설정
				FOnMontageEnded EndDelegate;
				EndDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
					{
						// 첫 번째 몽타주가 끝나면 Dracula 메시를 숨기고 Wolf 메시를 보이도록 전환
						D1Killer->GetBatMesh()->SetHiddenInGame(true);
						D1Killer->GetCharacterMesh()->SetHiddenInGame(false);
						D1Killer->GetFPVMesh()->SetHiddenInGame(false);

						D1Killer->SwitchCamera(ECreatureState::Dracula);

						// 두 번째 몽타주(Wolf_In_Dracula)를 Wolf 메시에서 재생
						UAnimInstance* DracAnimInst = D1Killer->GetCharacterMesh()->GetAnimInstance();
						if (DracAnimInst && Dracula_In_Bat_Montage)
						{
							DracAnimInst->Montage_Play(Dracula_In_Bat_Montage, 1.0f);
						}
						// 상태 전환 처리 (필요한 경우 CreatureState 업데이트)
						SetCreatureState(ECreatureState::Dracula);
					});
				BatAnimInst->Montage_SetEndDelegate(EndDelegate, Bat_Out_Dracula_Montage);
				return;
			}
		}
	}
}

ECreatureState AD1KillerController::GetCreatureState()
{
	if (D1Killer)
	{
		return D1Killer->CreatureState;
	}

	return ECreatureState::None;
}

void AD1KillerController::SetCreatureState(ECreatureState InState)
{
	if (D1Killer)
	{
		D1Killer->CreatureState = InState;
	}
}

void AD1KillerController::SwitchCameraToState(ECreatureState NewState, float BlendTime)
{
	if(!D1Killer)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchCameraToState: Pawn is not a KillerBase!"));
		return;
	}

	TObjectPtr<UCameraComponent> TargetCamComp = nullptr;

	switch (NewState)
	{
	case ECreatureState::Dracula:
		TargetCamComp = D1Killer->GetFirstPersonCameraComponent();
		break;
	case ECreatureState::Wolf:
		TargetCamComp = D1Killer->GetWolfCameraComponent();
		break;
	case ECreatureState::Bat:
		TargetCamComp = D1Killer->GetBatCameraComponent();
		break;
	}
	if (!TargetCamComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchCameraToState: Target camera component is null!"));
		return;
	}

	D1Killer->SwitchCamera(NewState);
	FTransform TargetCamTransform = TargetCamComp->GetComponentTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACameraActor* TempCam = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), TargetCamTransform, SpawnParams);
	if (!TempCam)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchCameraToState: Failed to spawn temporary camera actor!"));
		return;
	}

	if (UCameraComponent* TempCamComp = TempCam->GetCameraComponent())
	{
		TempCamComp->SetFieldOfView(TargetCamComp->FieldOfView);
	}

	EViewTargetBlendFunction BlendFunc = VTBlend_EaseInOut;
	float BlendExp = 2.0f;
	bool bLockOutgoing = false;
	SetViewTargetWithBlend(TempCam, BlendTime, BlendFunc, BlendExp, bLockOutgoing);

	AD1KillerBase* Killer = Cast<AD1KillerBase>(GetCharacter());
	// BlendTime 후에 뷰 타겟을 다시 KillerBase로 전환하고 임시 카메라 파괴
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, Killer, TempCam]()
		{
			SetViewTarget(Killer);
			if (TempCam)
			{
				TempCam->Destroy();
			}
		}), BlendTime, false);
}