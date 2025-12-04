// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "SkillSlotWidget.generated.h"

class UAbilitySystemComponent;

/**
*
 */
UCLASS()
class PROJECTAE_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void BindToASC(UAbilitySystemComponent* ASC, const FGameplayTag& InCoolDownTag);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void OnCooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

private:
	// 스킬 태그
	FGameplayTag CoolDownTag;

	// ASC 캐시
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

	float ColdownStartTime = 0.f;

	float DurationTime = 0.f;

	bool bCanTick = false;
};