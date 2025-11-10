// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_BaseStaminaAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AS_BaseCombat.h"


bool UGA_BaseStaminaAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return false;
	}

	const UAS_BaseCombat* AS = ASC->GetSet<UAS_BaseCombat>();
	if (!AS)
	{
	return false;
	}

	if (StaminaChunkCost <= AS->GetStamina() && StaminaChunkCostGE)
	{
		return true;
	}

	if (StaminaDrainPerSecond <= AS->GetStamina() && StaminaChunkCostGE)
	{
		return true;
	}
	return false;
}

void UGA_BaseStaminaAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}


	const FGameplayTag CostTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Cost"));
	if (StaminaChunkCostGE)
	{
		//FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaChunkCostGE)
		{
			//TODO : GE를 적용하고, SetByCaller로 적용된 Cost를 적용시키기
		}
	}

}

void UGA_BaseStaminaAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (DrainEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);

		ASC->GetGameplayAttributeValueChangeDelegate(UAS_BaseCombat::GetMaxStaminaAttribute()).Remove(StaminaChangedDelegateHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BaseStaminaAbility::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue < StaminaDrainPerSecond)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}