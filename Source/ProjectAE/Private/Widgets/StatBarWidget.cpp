// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/StatBarWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"

void UStatBarWidget::BindToASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("StatUIBar: InValid ASC Input in Function UStatBarWidget::BindToASC, Did you miss it?"));
		return;
	}

	WeakASC = ASC;

    bool bFoundCurrent;
    bool bFoundMax;

    CurrentValue = ASC->GetGameplayAttributeValue(CurrentAttribute, bFoundCurrent);
    MaxValue = ASC->GetGameplayAttributeValue(MaxAttribute, bFoundMax);

    // 델리게이트 바인딩
    if (bFoundCurrent)
    {
        ASC->GetGameplayAttributeValueChangeDelegate(CurrentAttribute)
            .AddUObject(this, &UStatBarWidget::OnCurrentValueChanged);
    }

    if (bFoundMax)
    {
        ASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute)
            .AddUObject(this, &UStatBarWidget::OnMaxValueChanged);
    }

    // 최초 UI 갱신
    UpdateProgressBar();

    if (GhostBar && MainBar)
    {
        GhostBar->SetPercent(MainBar->GetPercent());
    }
}

void UStatBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateBarColor();
}

void UStatBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 고스트 바 로직
    if (GhostBar && MainBar)
    {
        float CurrentGhost = GhostBar->GetPercent();
        float TargetMain = MainBar->GetPercent();

        if (CurrentGhost > TargetMain)
        {
            // 데미지 입음: 천천히 줄어듦
            float NewGhost = FMath::FInterpTo(CurrentGhost, TargetMain, InDeltaTime, GhostInterpSpeed);
            GhostBar->SetPercent(NewGhost);
        }
        else if (CurrentGhost < TargetMain)
        {
            // 회복됨: 즉시 따라감
            GhostBar->SetPercent(TargetMain);
        }
    }
}

void UStatBarWidget::OnCurrentValueChanged(const FOnAttributeChangeData& Data)
{
    CurrentValue = Data.NewValue;
    UpdateProgressBar();

    if (bHasEffectOnValueBecomeZero)
    {
        OnValueBecomeZero();
    }
}

void UStatBarWidget::OnMaxValueChanged(const FOnAttributeChangeData& Data)
{
    MaxValue = Data.NewValue;
    UpdateProgressBar();
}

void UStatBarWidget::UpdateProgressBar()
{
    if (MaxValue <= 0.0f) return; // 0 나누기 방지

    float StatPercent = FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f);

    if (MainBar)
    {
        MainBar->SetPercent(StatPercent);
    }
}

void UStatBarWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    UpdateBarColor();
}

void UStatBarWidget::UpdateBarColor()
{
    if (MainBar)
    {
        MainBar->SetFillColorAndOpacity(BarColor);
    }

    if (GhostBar)
    {
        GhostBar->SetFillColorAndOpacity(GhostBarColor);
    }
}
