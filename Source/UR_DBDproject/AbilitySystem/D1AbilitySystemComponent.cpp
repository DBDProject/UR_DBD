// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"

namespace EnhancedInputAbilitySystem_Impl
{
	constexpr int32 InvalidInputID = 0;
	int32 IncrementingInputID = InvalidInputID;

	static int32 GetNextInputID()
	{
		return ++IncrementingInputID;
	}
}

void UD1AbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& StartupAbilities)
{
    if (!StartupAbilities.Num()) return;

    for (auto& Ability : StartupAbilities)
    {
        if (Ability)
        {
            FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1);
            FGameplayAbilitySpecHandle SpecHandle = GiveAbility(AbilitySpec);
            SpecHandles.Add(SpecHandle);
        }
    }
}

void UD1AbilitySystemComponent::ActivateAbility(FGameplayTag AbilityTag)
{
    if (!AbilityTag.IsValid()) return;

    for (FGameplayAbilitySpecHandle& SpecHandle : SpecHandles)
    {
        FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle);
        if (Spec && Spec->Ability && Spec->Ability->AbilityTags.HasTagExact(AbilityTag))
        {
            TryActivateAbility(SpecHandle);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("🚨 Ability with tag [%s] not found!"), *AbilityTag.ToString());


}

void UD1AbilitySystemComponent::SetInputBinding(UInputAction* InputAction, FGameplayTag AbilityTag)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	// Gameplay Tag에 해당하는 AbilitySpec 찾기
	FGameplayAbilitySpec* BindingAbility = nullptr;
	for (FGameplayAbilitySpecHandle& SpecHandle : SpecHandles)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle);
		if (Spec->Ability && Spec->Ability->AbilityTags.HasTagExact(AbilityTag))
		{
			BindingAbility = Spec;
			break;
		}
	}

	if (!BindingAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetInputBinding: No ability found with tag %s"), *AbilityTag.ToString());
		return;
	}

	FAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);
	if (AbilityInputBinding)
	{
		// 기존에 바인딩된 Ability 찾기
		FGameplayAbilitySpec* OldBoundAbility = FindAbilitySpec(AbilityInputBinding->BoundAbilitiesStack.Top());
		if (OldBoundAbility && OldBoundAbility->InputID == AbilityInputBinding->InputID)
		{
			OldBoundAbility->InputID = InvalidInputID;
		}
	}
	else
	{
		// 새로운 Input 바인딩 추가
		AbilityInputBinding = &MappedAbilities.Add(InputAction);
		AbilityInputBinding->InputID = GetNextInputID();
	}

	if (BindingAbility)
	{
		BindingAbility->InputID = AbilityInputBinding->InputID;
	}

	AbilityInputBinding->BoundAbilitiesStack.Push(BindingAbility->Handle);
	TryBindAbilityInput(InputAction, *AbilityInputBinding);
}

void UD1AbilitySystemComponent::ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	if (FGameplayAbilitySpec* FoundAbility = FindAbilitySpec(AbilityHandle))
	{
		// Find the mapping for this ability
		auto MappedIterator = MappedAbilities.CreateIterator();
		while (MappedIterator)
		{
			if (MappedIterator.Value().InputID == FoundAbility->InputID)
			{
				break;
			}

			++MappedIterator;
		}

		if (MappedIterator)
		{
			FAbilityInputBinding& AbilityInputBinding = MappedIterator.Value();

			if (AbilityInputBinding.BoundAbilitiesStack.Remove(AbilityHandle) > 0)
			{
				if (AbilityInputBinding.BoundAbilitiesStack.Num() > 0)
				{
					FGameplayAbilitySpec* StackedAbility = FindAbilitySpec(AbilityInputBinding.BoundAbilitiesStack.Top());
					if (StackedAbility && StackedAbility->InputID == 0)
					{
						StackedAbility->InputID = AbilityInputBinding.InputID;
					}
				}
				else
				{
					// NOTE: This will invalidate the `AbilityInputBinding` ref above
					RemoveEntry(MappedIterator.Key());
				}
				// DO NOT act on `AbilityInputBinding` after here (it could have been removed)


				FoundAbility->InputID = InvalidInputID;
			}
		}
	}
}
void UD1AbilitySystemComponent::ClearAbilityBindings(UInputAction* InputAction)
{
	RemoveEntry(InputAction);
}

void UD1AbilitySystemComponent::OnAbilityInputPressed(UInputAction* InputAction)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
	if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
	{
		AbilityLocalInputPressed(FoundBinding->InputID);
	}
}

void UD1AbilitySystemComponent::OnAbilityInputReleased(UInputAction* InputAction)
{
	using namespace EnhancedInputAbilitySystem_Impl;

	FAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
	if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
	{
		AbilityLocalInputReleased(FoundBinding->InputID);
	}
}

void UD1AbilitySystemComponent::RemoveEntry(UInputAction* InputAction)
{
	if (FAbilityInputBinding* Bindings = MappedAbilities.Find(InputAction))
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		for (FGameplayAbilitySpecHandle AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace EnhancedInputAbilitySystem_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		MappedAbilities.Remove(InputAction);
	}
}

FGameplayAbilitySpec* UD1AbilitySystemComponent::FindAbilitySpec(FGameplayAbilitySpecHandle Handle)
{
	FGameplayAbilitySpec* FoundAbility = nullptr;
	FoundAbility = FindAbilitySpecFromHandle(Handle);
	return FoundAbility;
}

void UD1AbilitySystemComponent::TryBindAbilityInput(UInputAction* InputAction, FAbilityInputBinding& AbilityInputBinding)
{
	if (InputComponent)
	{
		// Pressed event
		if (AbilityInputBinding.OnPressedHandle == 0)
		{
			AbilityInputBinding.OnPressedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &UD1AbilitySystemComponent::OnAbilityInputPressed, InputAction).GetHandle();
		}

		// Released event
		if (AbilityInputBinding.OnReleasedHandle == 0)
		{
			AbilityInputBinding.OnReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UD1AbilitySystemComponent::OnAbilityInputReleased, InputAction).GetHandle();
		}
	}
}

void UD1AbilitySystemComponent::BeginPlay() {
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	if (IsValid(Owner) && Owner->InputComponent) {
		InputComponent = CastChecked<UEnhancedInputComponent>(Owner->InputComponent);
	}
}