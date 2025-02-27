#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "D1AnimNotify_SendGameplayEvent.generated.h"

/**
 *
 */
UCLASS()
class UR_DBDPROJECT_API UD1AnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UD1AnimNotify_SendGameplayEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag EventTag;
};