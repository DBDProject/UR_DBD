#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "D1Define.h"
#include "D1Hook.generated.h"

UCLASS()
class UR_DBDPROJECT_API AD1Hook : public AActor
{
    GENERATED_BODY()

public:
    AD1Hook();

protected:
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual void Tick(float DeltaTime) override;

    void ActivateEntity();
    void DeactivateEntity();

    UFUNCTION(NetMulticast, Reliable)
    void StartDissolveEffect(class AD1SurvivorBase* Player);
    void PlayEntityMontage(FName Section);

    UFUNCTION(BlueprintCallable)
    void StartHookCameraCutscene();
    UFUNCTION(BlueprintCallable)
    void EndHookCameraCutscene();
    UFUNCTION(BlueprintCallable)
    void PlayHookExecutionSequence();

    void RemoveFromGame();

    UFUNCTION(NetMulticast, Reliable)
    void StartDissolveDisappearEffect(); // 엔티티가 점점 사라지는 함수

private:
    void UpdateDissolve();
    void UpdateDissolveDisappear();
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USceneComponent> RootScene;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UCineCameraComponent> HookReactionCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class ULevelSequence> HookExecutionSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class ACineCameraActor> ExecutionCameraActor;

    // 상호작용 범위 콜라이더
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class UBoxComponent> InteractionBox;

    // 갈고리 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class USkeletalMeshComponent> HookMesh;

    // 엔티티 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class USkeletalMeshComponent> EntityMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<class UAnimMontage> EntityMontage; // 엔티티 몽타주

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class AD1SurvivorBase> InteractingPlayer;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMat_Slot0;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMat_Slot1;

    UPROPERTY(Replicated)
    float CurrentDissolveValue;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Hook", meta = (AllowPrivateAccess = "true"))
    bool bIsSkillCheckEnable = false;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Hook", meta = (AllowPrivateAccess = "true"))
    bool bIsSkillCheckFail = false;

    UPROPERTY(Replicated)
    bool bEntityVisible;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Hook")
    bool bIsHooked = false;

    FTimerHandle DissolveTimer;

    float DissolveStartTime;

public:
    TObjectPtr<class USkeletalMeshComponent> GetHookMesh() { return HookMesh; }
    TObjectPtr<class USkeletalMeshComponent> GetEntityMesh() { return EntityMesh; }
    void SetInteractingPlayer(class AD1SurvivorBase* Player) { InteractingPlayer = Player; }
    void SetIsSkillCheckEnable(bool State) { bIsSkillCheckEnable = State; }
    void SetIsSkillCheckFail(bool State) { bIsSkillCheckFail = State; }
    void SetIsHooked(bool State) { bIsHooked = State; }
    bool GetEntityVisible() { return bEntityVisible; }
};
