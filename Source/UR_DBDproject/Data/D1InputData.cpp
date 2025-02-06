// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/D1InputData.h"
#include "D1LogChannels.h"

const UInputAction* UD1InputData::FindInputActionByTag(const FGameplayTag& InputTag) const
{
	for (const FD1InputAction& Action : InputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	UE_LOG(LogD1, Error, TEXT("Can't find InputAction for InputTag [%s]"), *InputTag.ToString());

	return nullptr;
}