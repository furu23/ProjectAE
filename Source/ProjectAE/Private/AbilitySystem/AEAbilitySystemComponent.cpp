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
