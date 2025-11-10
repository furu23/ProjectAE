// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInputConfig.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class PROJECTAE_API UAbilityInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 어빌리티 시스템에 바인딩할 인풋 액션과 태그입니다.
	// TMap 키: UInputAction*, 값: FGameplayTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Input", meta = (DisplayName = "Ability Input Actions"))
	TMap<TObjectPtr<UInputAction>, FGameplayTag> AbilityInputActions;
};
