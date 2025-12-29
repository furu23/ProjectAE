// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AS_HealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

#define MINIMUM_MAXHEALTH 10.f


UAS_HealthSet::UAS_HealthSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UAS_HealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == UAS_HealthSet::GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == UAS_HealthSet::GetMaxHealthAttribute())
	{
		// 최소 최대 체력 값을 10으로 제한합니다.
        NewValue = FMath::Max(NewValue, MINIMUM_MAXHEALTH);
	}
}

void UAS_HealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

//     if (Attribute == GetHealthAttribute())
//     {
//         float CurrentHealth = GetHealth();
//         float CurrentBaseHealth = GetHealthAttribute().GetNumericValue(this);
//         float CurrentModifiers = CurrentHealth - CurrentBaseHealth;
// 
//         if (CurrentHealth > NewValue)
//         {
//             UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
//             if (ASC)
//             {
// 
//                 float NewBaseHealth = NewValue - CurrentModifiers;
// 
//                 ASC->SetNumericAttributeBase(GetHealthAttribute(), NewBaseHealth);
//             }
//         }
//     }
}

void UAS_HealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
}

// void UAS_HealthSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
// {
// 
// }
// 
// void UAS_HealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
// {
// 
// }
