// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/D1AnimNotify_SendGameplayEvent.h"
#include "Characters/D1CharacterBase.h"

UD1AnimNotify_SendGameplayEvent::UD1AnimNotify_SendGameplayEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UD1AnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AD1CharacterBase* LocalCharacter = Cast<AD1CharacterBase>(MeshComp->GetOwner());
	if (LocalCharacter)
	{
		LocalCharacter->HandleGameplayEvent(EventTag);
	}
}
