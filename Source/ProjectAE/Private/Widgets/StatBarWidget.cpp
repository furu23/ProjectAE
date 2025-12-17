// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/StatBarWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UStatBarWidgetBase::BindToASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("StatUIBar: InValid ASC Input in Function UStatBarWidgetBase::BindToASC, Did you miss it?"));
		return;
	}

    SetVisibility(ESlateVisibility::HitTestInvisible);


	WeakASC = ASC;

    bool bFoundCurrent;
    bool bFoundMax;

    TargetValue = ASC->GetGameplayAttributeValue(CurrentAttribute, bFoundCurrent);
    MaxValue = ASC->GetGameplayAttributeValue(MaxAttribute, bFoundMax);

    CurrentDisplayedValue = TargetValue;

    // 델리게이트 바인딩
    if (ensure(bFoundCurrent))
    {
        ASC->GetGameplayAttributeValueChangeDelegate(CurrentAttribute)
            .AddUObject(this, &UStatBarWidgetBase::OnCurrentValueChanged);
    }

    if (ensure(bFoundMax))
    {
        ASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute)
            .AddUObject(this, &UStatBarWidgetBase::OnMaxValueChanged);
    }

    // 첫 그리기
    bIsAnimating = true;

    // 동기화 성공 시 호출하는 API
    OnPostBindToASC();
}

void UStatBarWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (bAutoFadeOut)
    {
        SetRenderOpacity(0.0f);
    }
    else
    {
        SetRenderOpacity(1.0f);
    }
}

void UStatBarWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsAnimating)
    {
        CurrentDisplayedValue = FMath::FInterpTo(CurrentDisplayedValue, TargetValue, InDeltaTime, InterpSpeed);
        
        if (FMath::IsNearlyEqual(CurrentDisplayedValue, TargetValue, 0.1f))
        {
            CurrentDisplayedValue = TargetValue;
            bIsAnimating = false;
        }

        UpdateVisuals(CurrentDisplayedValue, MaxValue);
    }

    if (bAutoFadeOut)
    {
        if (bIsFadingOut)
        {
            float CurrentOpacity = GetRenderOpacity();

            float NewOpacity = FMath::FInterpConstantTo(CurrentOpacity, 0.0f, InDeltaTime, InterpOpacitySpeed);
            SetRenderOpacity(NewOpacity);

            if (NewOpacity <= 0.0f)
            {
                bIsFadingOut = false;
            }
        }
    }
}

void UStatBarWidgetBase::OnCurrentValueChanged(const FOnAttributeChangeData& Data)
{
    TargetValue = Data.NewValue;

    NotifyChange();

    if (TargetValue <= 0.0f && bHasEffectOnValueBecomeZero)
    {
        OnValueBecomeZero();
    }

    if (TargetValue >= MaxValue && bHasEffectOnValueBecomeFull)
    {
        OnValueBecomeFull();
    }

    bIsAnimating = true;

    K2_OnCurrentValueChanged(Data.NewValue, Data.OldValue);
}

void UStatBarWidgetBase::OnMaxValueChanged(const FOnAttributeChangeData& Data)
{
    MaxValue = Data.NewValue;

    bIsAnimating = true;

    K2_OnMaxValueChanged(Data.NewValue, Data.OldValue);
}

void UStatBarWidgetBase::NotifyChange()
{
    if (!bAutoFadeOut)
    {
        SetRenderOpacity(1.0f);
        return;
    }

    bIsFadingOut = false;
    SetRenderOpacity(1.0f);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            HideTimerHandle,
            this,
            &UStatBarWidgetBase::StartFadeOut,
            HideDelayTime,
            false
        );
    }
}

void UStatBarWidgetBase::StartFadeOut()
{
    bIsFadingOut = true;
}


// **** URaidalStatBarWidget ****


void URadialStatBarWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (RadialImage)
    {
        MaterialDynamic = RadialImage->GetDynamicMaterial();
    }
}

void URadialStatBarWidget::UpdateVisuals(float DisplayValue, float MaxStatValue)
{
    if (MaxValue <= KINDA_SMALL_NUMBER) return;

    float Percent = FMath::Clamp(DisplayValue / MaxValue, 0.0f, 1.0f);

    if (MaterialDynamic)
    {
        MaterialDynamic->SetScalarParameterValue(PercentParamName, Percent);
    }
}

// **** ULinearStatBarWidget ****

void ULinearStatBarWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UpdateBarColor();
}

void ULinearStatBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bEnableGhostBar && GhostBar && MainBar)
    {
        float TargetPercent = MainBar->GetPercent();
        float CurrentGhost = GhostBar->GetPercent();

        if (CurrentGhost > TargetPercent)
        {
            if (!FMath::IsNearlyEqual(CurrentGhost, TargetPercent, 0.001f))
            {
                float NewGhost = FMath::FInterpTo(CurrentGhost, TargetPercent, InDeltaTime, GhostInterpSpeed);
                GhostBar->SetPercent(NewGhost);
            }
            else
            {
                GhostBar->SetPercent(TargetPercent);
            }
        }
    }
}

void ULinearStatBarWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();
    UpdateBarColor();
}

void ULinearStatBarWidget::UpdateVisuals(float DisplayValue, float MaxStatValue)
{
    if (MaxStatValue <= 0.0f) return; // 0 나누기 방지

    float StatPercent = FMath::Clamp(DisplayValue / MaxStatValue, 0.0f, 1.0f);

    if (MainBar)
    {
        MainBar->SetPercent(StatPercent);
    }

    if (bEnableGhostBar && GhostBar)
    {
        if (GhostBar->GetPercent() < StatPercent)
        {
            GhostBar->SetPercent(StatPercent);
        }
    }
}

void ULinearStatBarWidget::UpdateBarColor()
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
