// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AS_BaseCombat.h"
#include "Net/UnrealNetwork.h"

void UAS_BaseCombat::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{

}

void UAS_BaseCombat::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{

}

void UAS_BaseCombat::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{

}

void UAS_BaseCombat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseCombat, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseCombat, MaxHealth, COND_None, REPNOTIFY_Always);

}

void UAS_BaseCombat::OnRep_Health(const FGameplayAttributeData& OldHealth)
{

}

void UAS_BaseCombat::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{

}

void UAS_BaseCombat::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{

}

void UAS_BaseCombat::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{

}

void UAS_BaseCombat::OnRep_Bio(const FGameplayAttributeData& OldBio)
{

}

void UAS_BaseCombat::OnRep_MaxBio(const FGameplayAttributeData& OldMaxBio)
{

}

void UAS_BaseCombat::OnRep_BaseAttack(const FGameplayAttributeData& AEBaseAttack)
{

}

void UAS_BaseCombat::OnRep_BaseDefense(const FGameplayAttributeData& OldBaseDefense)
{

}