// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AS_HealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

#define MINIMUM_HEALTH 10.f

UAS_HealthSet::UAS_HealthSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UAS_HealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == UAS_HealthSet::GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(NewValue, 0.f, GetMaxHealth()));
	}
	else if (Attribute == UAS_HealthSet::GetMaxHealthAttribute())
	{
		// 최소 최대 체력 값을 10으로 제한합니다.
		SetMaxHealth(FMath::Clamp(NewValue, MINIMUM_HEALTH, GetMaxHealth()));
	}
}

void UAS_HealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	if (Attribute == UAS_HealthSet::GetMaxHealthAttribute())
	{
		if (GetHealth() > NewValue)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ASC)
			{
				ASC->SetNumericAttributeBase(GetHealthAttribute(), NewValue);
			}
		}
	}
}

void UAS_HealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if (GetHealth() <= 0)
		{
			// 캐릭터 죽음 관련 로직 실행
		}
	}
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
