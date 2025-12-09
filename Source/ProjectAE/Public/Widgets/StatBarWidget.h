// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "StatBarWidget.generated.h"

class UProgressBar;

/**
 * @brief 각종 스텟의 변화량을 확인할 수 있는 기반 위젯
 * 
 * @note 기능적인 요소들을 포함하며 디자인적 요소는 확장 가능
 */
UCLASS()
class PROJECTAE_API UStatBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 위젯이 생성될 때가 아니라, 실제 캐릭터(ASC)와 연결될 때 호출할 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void BindToASC(UAbilitySystemComponent* ASC);


	// BP 확장을 위한 NVI 적용 함수들
	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnCurrentValueChanged"))
	void K2_OnCurrentValueChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnMaxValueChanged"))
	void K2_OnMaxValueChanged(float NewValue, float OldValue);


protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 값이 변했을 때 호출될 콜백
	void OnCurrentValueChanged(const FOnAttributeChangeData& Data);
	void OnMaxValueChanged(const FOnAttributeChangeData& Data);

	// 값이 0일때 호출될 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnValueBecomeZero();

	// UI 업데이트 헬퍼
	UFUNCTION(BlueprintCallable)
	void UpdateProgressBar();

	// 작업을 확인할 수 있는 동기화용
	virtual void SynchronizeProperties() override;

public:
	// 에디터에서 어트리뷰트를 선택
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayAttribute CurrentAttribute;

	// Max 값도 가져오기 위해 필요
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayAttribute MaxAttribute;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> MainBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> GhostBar;

	// **** 설정 ****
	UPROPERTY(EditAnywhere, Category = "UI")
	float GhostInterpSpeed = 3.0f;

	// 0이 될 시 효과 적용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bHasEffectOnValueBecomeZero = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Configuration")
    FLinearColor BarColor = FLinearColor::Green; // 기본값 초록

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Configuration")
    FLinearColor GhostBarColor = FLinearColor::Gray; // 기본값 회색

private:
	// 현재 값 캐싱용
	float CurrentValue = 0.0f;
	float MaxValue = 1.0f;


	// ASC에 대한 약한 참조 (안전성)
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

	// 색상 변경 함수
	void UpdateBarColor();
};