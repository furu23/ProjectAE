// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/StatBarContainerWidget.h"
#include "Widgets/StatBarWidget.h"
#include "Components/PanelWidget.h"
#include "AbilitySystemComponent.h"

void UStatBarContainerWidget::SetAbilitySystemComponent(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("StatUIBar: InValid ASC Input in Function UStatBarWidget::BindToASC, Did you miss it?"));
		return;
	}

	WeakASC = ASC;
}

void UStatBarContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 배열 초기화
	StatBars.Empty();

	if (BarContainer)
	{
		// 컨테이너(VerticalBox)의 모든 자식 위젯을 순회합니다.
		int32 ChildCount = BarContainer->GetChildrenCount();

		for (int32 i = 0; i < ChildCount; ++i)
		{
			UWidget* ChildWidget = BarContainer->GetChildAt(i);

			// 자식이 우리가 만든 'GenericStatBar'라면 배열에 추가
			if (UStatBarWidget* Bar = Cast<UStatBarWidget>(ChildWidget))
			{
				if (ensureMsgf(WeakASC.IsValid(), TEXT("InValid ASC in StatBarContainer. Is it Initialize Correctly?")))
				{
					Bar->BindToASC(WeakASC.Get());
				}

				StatBars.Add(Bar);
			}
		}
	}
}
