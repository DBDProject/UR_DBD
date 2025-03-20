// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Data/D1InputData.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "D1GameplayTags.h"
#include "D1AbilitySystemComponent.generated.h"

class UInputAction;

USTRUCT()
struct FAbilityInputBinding
{
	GENERATED_BODY()

	int32  InputID = 0;
	uint32 OnPressedHandle = 0;
	uint32 OnReleasedHandle = 0;
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;
};

/**
 * 
 */
UCLASS()
class UR_DBDPROJECT_API UD1AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& StartupAbilities);

	void ActivateAbility(FGameplayTag AbilityTag);

	TArray<FGameplayAbilitySpecHandle> SpecHandles;

public:
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void SetInputBinding(UInputAction* InputAction, FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Abilities")
	void ClearAbilityBindings(UInputAction* InputAction);

public:
	void OnAbilityInputPressed(UInputAction* InputAction);

	void OnAbilityInputReleased(UInputAction* InputAction);

	void RemoveEntry(UInputAction* InputAction);

	void TryBindAbilityInput(UInputAction* InputAction, FAbilityInputBinding& AbilityInputBinding);

	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle);

	virtual void BeginPlay() override;

	UPROPERTY(transient)
	TMap<UInputAction*, FAbilityInputBinding> MappedAbilities;

	UPROPERTY(transient)
	UEnhancedInputComponent* InputComponent;
};
