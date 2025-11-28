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

	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UAS_BaseCombat* AS = ASC ? Cast<UAS_BaseCombat>(ASC->GetSet<UAS_BaseCombat>()) : nullptr;
	if (!AS)
	{
		return false;
	}

	const float CurrentStamina = AS->GetStamina();
	if (StaminaChunkCostGE)
	{
		return StaminaChunkCost + CurrentStamina >= 0;
	}

	if (StaminaDrainCostGE)
	{
		return StaminaDrainPerSecond <= CurrentStamina;
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

	// 단발성 스테미나 소모 GE 적용
	if (StaminaChunkCostGE)
	{
		// 적용시킬 GE SpecHandler를 컨텍스트와 함께 생성
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaChunkCostGE);
		if (SpecHandle.IsValid())
		{
			//GE를 적용하고, SetByCaller로 적용된 Cost를 적용시키기
			SpecHandle.Data->SetSetByCallerMagnitude(CostTag, StaminaChunkCost);

			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}

	// 지속성 스테미나 소모 GE 적용
	if (StaminaDrainCostGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaDrainCostGE);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(CostTag, StaminaDrainPerSecond);
			// GE 적용 및 핸들 저장 (지속성이므로)
			DrainEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);

			// 스테미나가 0이 되면 어빌리티 취소용 델리게이트 바인딩
			StaminaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UAS_BaseCombat::GetStaminaAttribute()).AddUObject(this, &UGA_BaseStaminaAbility::OnStaminaChanged);
		}
	}
}

void UGA_BaseStaminaAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (DrainEffectHandle.IsValid())
	{
		// 지속성 스테미나 소모 GE 제거
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);

		// 델리게이트 해제
		ASC->GetGameplayAttributeValueChangeDelegate(UAS_BaseCombat::GetStaminaAttribute()).Remove(StaminaChangedDelegateHandle);
	}

	// 기본 EndAbility 호출, 항상 마지막에.
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BaseStaminaAbility::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	// 스테미나가 0이 되면 어빌리티 취소
	if (Data.NewValue <= 0.f)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}