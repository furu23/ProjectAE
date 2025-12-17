// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AS_HealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Core/AEGlobalHelper.h"

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
		NewValue = FMath::Clamp(NewValue, MINIMUM_MAXHEALTH, GetMaxHealth());
	}
}

void UAS_HealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        // 1. 현재 적용된 Modifier 값 계산 (최종값 - 기본값)
        // 주의: 이 시점에서 GetHealth()는 아직 클램핑 전의 값을 리턴할 수 있음
        float CurrentHealth = GetHealth();
        float CurrentBaseHealth = GetHealthAttribute().GetNumericValue(this);
        float CurrentModifiers = CurrentHealth - CurrentBaseHealth;

        // 2. 만약 (현재 체력 > 새로운 최대 체력) 이라면?
        if (CurrentHealth > NewValue)
        {
            UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
            if (ASC)
            {
                // 3. Modifier를 고려하여 Base 값을 설정
                // 목표: Base + Modifiers = NewMaxHealth
                // 따라서: Base = NewMaxHealth - Modifiers
                float NewBaseHealth = NewValue - CurrentModifiers;

                // Base 값을 수정하여 최종 값이 NewValue(MaxHealth)와 같게 만듦
                ASC->SetNumericAttributeBase(GetHealthAttribute(), NewBaseHealth);
            }
        }
    }
}

void UAS_HealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
}

void UAS_HealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAS_HealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_HealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
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
