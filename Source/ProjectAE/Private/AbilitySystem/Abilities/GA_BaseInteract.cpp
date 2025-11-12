// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_BaseInteract.h"
#include "GameplayAbilitySpec.h"

AActor* UGA_BaseInteract::GetTargetActorFromSpec() const
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec)
	{
		return nullptr;
	}

	if (Spec->SourceObject.Get())
	{
		return Cast<AActor>(Spec->SourceObject.Get());
	}
	return nullptr;
}

void UGA_BaseInteract::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CachedTargetActor = GetTargetActorFromSpec();
	if (!CachedTargetActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

bool UGA_BaseInteract::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /* = nullptr */, const FGameplayTagContainer* TargetTags /* = nullptr */, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const
{

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}
