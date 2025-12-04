// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AEAbilitySystemComponent.h"

void UAEAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// 해당 태그와 매핑된 어빌리티들을 활성화합니다.
		this->AddLooseGameplayTag(InputTag);
	}
}

void UAEAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// 해당 태그와 매핑된 어빌리티들을 비활성화합니다.
		this->RemoveLooseGameplayTag(InputTag);
	}
}

bool UAEAbilitySystemComponent::GetGameplayEffectDurationAndTimeRemainingFromTag(FGameplayTag CooldownTag, float& OutDuration, float& OutTimeRemaining) const
{
    if (!CooldownTag.IsValid()) { return false; }

    // Query 없이 전체 순회 (가장 확실함)
    for (const FActiveGameplayEffectHandle& Handle : GetActiveGameplayEffects().GetAllActiveEffectHandles())
    {
        const FActiveGameplayEffect* ActiveEffect = GetActiveGameplayEffect(Handle);
        if (!ActiveEffect) continue;

        bool bHasTag = ActiveEffect->Spec.Def->InheritableGameplayEffectTags.CombinedTags.HasTag(CooldownTag);

        if (!bHasTag)
        {
            FGameplayTagContainer AllTags;
            ActiveEffect->Spec.GetAllGrantedTags(AllTags);
            if (AllTags.HasTag(CooldownTag))
            {
                bHasTag = true;
            }
        }

        if (bHasTag)
        {
            OutDuration = ActiveEffect->GetDuration();
            OutTimeRemaining = ActiveEffect->GetTimeRemaining(GetWorld()->GetTimeSeconds());
            return true;
        }
    }
    return false;
}