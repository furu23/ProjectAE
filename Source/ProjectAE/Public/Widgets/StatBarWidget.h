// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "Engine/TimerHandle.h"
#include "StatBarWidget.generated.h"

class UProgressBar;
class UImage;
class UWidgetAnimation;

/**
 * @brief 각종 스텟의 변화량을 확인할 수 있는 기반 위젯
 * 
 * @note 기능적인 요소들을 포함하며 디자인적 요소는 확장 가능
 */
UCLASS(Abstract)
class PROJECTAE_API UStatBarWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// 위젯이 생성될 때가 아니라, 실제 캐릭터(ASC)와 연결될 때 호출할 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void BindToASC(UAbilitySystemComponent* ASC);


	// UI가 줄어드는 속도
	UPROPERTY(EditAnywhere, Category = "UI")
	float InterpSpeed = 5.0f; // 메인 바가 줄어드는 속도

	// 에디터에서 어트리뷰트를 선택
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayAttribute CurrentAttribute;

	// Max 값도 가져오기 위해 필요
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayAttribute MaxAttribute;

protected:
	// **** 제공 API ****

	// 그리기를 구현하기 위한 순수 가상 함수
	virtual void UpdateVisuals(float DisplayValue, float MaxStatValue) PURE_VIRTUAL(UStatBarWidgetBase::UpdateVisuals, );
	// BP 확장을 위한 NVI 적용 함수들
	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnCurrentValueChanged"))
	void K2_OnCurrentValueChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnMaxValueChanged"))
	void K2_OnMaxValueChanged(float NewValue, float OldValue);

	// 값이 0일때 호출될 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnValueBecomeZero();

	// 값이 꽉 찼을 때 호출
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnValueBecomeFull();

    // 바인딩이 끝나고 값이 최초 동기화된 직후
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnPostBindToASC();


	// 0이 될 시 효과 적용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Option")
	bool bHasEffectOnValueBecomeZero = false;

	// 가득 찰 시 효과 적용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Option")
	bool bHasEffectOnValueBecomeFull = false;


	// **** Visibility 옵션 ****
	// true: 값이 변할 때만 잠깐 나타났다가 사라짐 (오버헤드 바)
	// false: 항상 떠 있음 (HUD)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Visibility")
	bool bAutoFadeOut = true; 

	// 페이드 아웃 진행 중인지 체크하는 bool 프로퍼티
	bool bIsFadingOut = false;

	UPROPERTY(EditAnywhere, Category = "UI|Visibility", meta=(EditCondition="bAutoFadeOut"))
	float HideDelayTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "UI|Visibility", meta=(EditCondition="bAutoFadeOut"))
	float InterpOpacitySpeed = 2.0f;


	// 데미지 혹은 소모 시에 호출될 함수
    void NotifyChange();

	// 타이머 콜백
	void StartFadeOut();


	// **** 내부 구현 관련 ****
	
	// 내부 로직
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void OnCurrentValueChanged(const FOnAttributeChangeData& Data);
	virtual void OnMaxValueChanged(const FOnAttributeChangeData& Data);



	// 현재 값 캐싱용
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float TargetValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float MaxValue = 1.0f;

	// 화면에 보여질 보간된 값
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float CurrentDisplayedValue = 0.0f;



private:
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;

	bool bIsAnimating = false;

	FTimerHandle HideTimerHandle;
};



UCLASS()
class PROJECTAE_API URadialStatBarWidget : public UStatBarWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	
    // Base 구현: 계산된 값을 머티리얼에 전달
	virtual void UpdateVisuals(float DisplayValue, float MaxStatValue) override;

protected:
	// 머티리얼이 적용된 이미지 위젯
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> RadialImage;

	// 머티리얼 내의 스칼라 파라미터 이름 (예: "Percent")
	UPROPERTY(EditAnywhere, Category = "Visuals")
	FName PercentParamName = FName("Percent");

private:
	// 런타임에 파라미터를 변경하기 위한 동적 머티리얼 인스턴스
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MaterialDynamic;
};



UCLASS()
class PROJECTAE_API ULinearStatBarWidget : public UStatBarWidgetBase
{
	GENERATED_BODY()


protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void SynchronizeProperties() override;

	// 계산된 값을 받아 UI에 적용
	virtual void UpdateVisuals(float DisplayValue, float MaxStatValue) override;



	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> MainBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> GhostBar;

	// **** 설정 ****
	UPROPERTY(EditAnywhere, Category = "UI|Ghost")
	bool bEnableGhostBar = false;

	UPROPERTY(EditAnywhere, Category = "UI|Ghost")
	float GhostInterpSpeed = 3.0f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Configuration")
    FLinearColor BarColor = FLinearColor::Red; // 기본값 빨강

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Configuration")
    FLinearColor GhostBarColor = FLinearColor::Gray; // 기본값 회색


private:

	// 색상 변경 함수
	void UpdateBarColor();
};