// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AS_BaseCombat.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Misc/AssertionMacros.h"

UAS_BaseCombat::UAS_BaseCombat()
{
	// AEGameplayTag 적용 시 제거
	StaminaRegenTag = FGameplayTag::RequestGameplayTag(TEXT("State.Stamina.Regenerating"));

	InitBio(20.f);
	InitMaxBio(20.f);

	InitStamina(50.f);
	InitMaxStamina(50.f);

	InitBaseAttack(1.f);
	InitBaseDefense(1.f);
}

void UAS_BaseCombat::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UAS_BaseCombat::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	if ( (Attribute == GetMaxBioAttribute())
		 || (Attribute == GetBaseAttackAttribute())
		 || (Attribute == GetBaseDefenseAttribute()) )
	{
		ensureMsgf(false, TEXT("Changing %s attribute is not planned for this project. Did you really need it?"), *Attribute.AttributeName);
		NewValue = OldValue;
	}
	else
	{
		// 0이하만 되지 않도록 제어
		NewValue = FMath::Max(0.f, NewValue);
	}
}

void UAS_BaseCombat::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetBioAttribute())
	{
		if (GetBio() <= 0)
		{
			// 초당 데미지 GE 등, Bio 소진 페널티 GE 부여
		}
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();

		if (GetStamina() < GetMaxStamina())
		{
			if (!bIsRegenerating)
			{
				ASC->AddLooseGameplayTag(StaminaRegenTag);
				bIsRegenerating = true;
			}
		}
		else if (GetStamina() >= GetMaxStamina())
		{
			ASC->RemoveLooseGameplayTag(StaminaRegenTag);
			bIsRegenerating = false;
		}
	}
}

void UAS_BaseCombat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseCombat, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseCombat, MaxStamina, COND_None, REPNOTIFY_Always);

}

// void UAS_BaseCombat::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
// {
// 
// }
// 
// void UAS_BaseCombat::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
// {
// 
// }
// 
// void UAS_BaseCombat::OnRep_Bio(const FGameplayAttributeData& OldBio)
// {
// 
// }
// 
// void UAS_BaseCombat::OnRep_MaxBio(const FGameplayAttributeData& OldMaxBio)
// {
// 
// }
// 
// void UAS_BaseCombat::OnRep_BaseAttack(const FGameplayAttributeData& AEBaseAttack)
// {
// 
// }
// 
// void UAS_BaseCombat::OnRep_BaseDefense(const FGameplayAttributeData& OldBaseDefense)
// {
//
// }