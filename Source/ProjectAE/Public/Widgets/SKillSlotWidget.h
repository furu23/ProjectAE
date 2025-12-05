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
	// 쿨다운에 관련된 위젯 바인딩을 처리, 반드시 시작하기 전에 호출해야 함
	UFUNCTION(BlueprintCallable, Category = "GAS", meta = (AutoCreateRefTerm = "InCoolDownTag"))
	void BindToASC(UAbilitySystemComponent* ASC, const FGameplayTag& InCoolDownTag);


	// 쿨다운 종료 및 시작 시 구현을 위한 API 제공
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnCoolDownStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnCoolDownEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnCoolDownProgressed();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void NativeDestruct() override;

	virtual void OnCooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// 스킬 태그
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	FGameplayTag CoolDownTag;


	// ASC 캐시
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;


	// 쿨다운 관련 값 변수들
	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float CooldownEndTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float RemainingTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float DurationTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float CooldownPercent = 0.f; // 0.0 ~ 1.0 (Progress Bar용)


	// Tick 활성화에 관한 변수
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	bool bCanTick = false;

private:
	// 델리게이트 관리를 위해
	FDelegateHandle SkillDelegateHandle;
};