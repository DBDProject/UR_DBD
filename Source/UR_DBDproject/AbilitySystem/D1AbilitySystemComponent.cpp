// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/D1AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/D1GameplayAbility.h"

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