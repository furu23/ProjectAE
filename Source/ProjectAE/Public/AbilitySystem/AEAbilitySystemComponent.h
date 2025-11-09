// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AEAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API UAEAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// 인풋 태그가 눌렸을 때 호출되는 함수
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	// 인풋 태그가 떼졌을 때 호출되는 함수
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	// TODO: GrantAbilitySet 헬퍼 함수 작성
};
