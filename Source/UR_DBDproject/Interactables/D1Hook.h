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

public:
    virtual void Tick(float DeltaTime) override;

    // 엔티티 효과 업데이트 (Dissolve 및 Transform 적용)
    void UpdateEntityEffect(float HookHealth);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USceneComponent> RootScene;

    // 상호작용 범위 콜라이더
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class UBoxComponent> InteractionBox;

    // 갈고리 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class USkeletalMeshComponent> HookMesh;

    // 엔티티 메쉬
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class USkeletalMeshComponent> EntityMesh;

    // 엔티티 머티리얼 인스턴스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hook")
    TObjectPtr<class UMaterialInstance> EntityDissolveMaterial;

public:
    TObjectPtr<class USkeletalMeshComponent> GetHookMesh() { return HookMesh; }
};
