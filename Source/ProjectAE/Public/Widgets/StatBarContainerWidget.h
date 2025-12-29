// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatBarContainerWidget.generated.h"

class UAbilitySystemComponent;
class UStatBarWidgetBase;

/**
 * @brief 체력 바 위젯들을 관리하는 컨테이너 객체입니다.
 * 
 * @see SetAbilitySystemComponent 외부에서 호출하여 반드시 ASC를 주입해주어야 합니다.
 */
UCLASS()
class PROJECTAE_API UStatBarContainerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 외부에서 호출하는 함수
	void SetAbilitySystemComponent(UAbilitySystemComponent* ASC);

protected:
	virtual void NativeConstruct() override;

protected:
	// 이것만 BindWidget으로 연결하면 됩니다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> BarContainer;

private:
	// UPROPERTY()를 붙여야 가비지 컬렉션(GC)에서 안전합니다.
	UPROPERTY()
	TArray<TObjectPtr<UStatBarWidgetBase>> StatBars;

	// ASC에 대한 약한 참조
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC;
};
