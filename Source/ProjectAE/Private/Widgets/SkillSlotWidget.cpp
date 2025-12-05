// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/SkillSlotWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"

void USkillSlotWidget::BindToASC(UAbilitySystemComponent* ASC, const FGameplayTag& InCoolDownTag)
{
	if (SkillDelegateHandle.IsValid() && WeakASC.IsValid())
	{
		WeakASC.Get()->RegisterGameplayTagEvent(CoolDownTag, EGameplayTagEventType::NewOrRemoved).Remove(SkillDelegateHandle);
	}

	WeakASC = ASC;
	CoolDownTag = InCoolDownTag;

	if (WeakASC.IsValid() && CoolDownTag.IsValid())
	{
		SkillDelegateHandle = ASC->RegisterGameplayTagEvent(CoolDownTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &USkillSlotWidget::OnCooldownTagChanged);
	}
	
}

void USkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bCanTick) { return; }

	float CurrentTime = GetWorld()->GetTimeSeconds();

	RemainingTime = FMath::Max(0.f, CooldownEndTime - CurrentTime);

	if (DurationTime > 0.f)
	{
		CooldownPercent = RemainingTime / DurationTime;
	}
	else
	{
		CooldownPercent = 0.f;
	}

	OnCoolDownProgressed();
}

void USkillSlotWidget::NativeDestruct()
{
	if (SkillDelegateHandle.IsValid() && WeakASC.IsValid())
	{
		WeakASC.Get()->RegisterGameplayTagEvent(CoolDownTag, EGameplayTagEventType::NewOrRemoved).Remove(SkillDelegateHandle);
	}

	Super::NativeDestruct();
}

void USkillSlotWidget::OnCooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		UAEAbilitySystemComponent* ASC = Cast<UAEAbilitySystemComponent>(WeakASC.Get());

		if (ASC)
		{
			float TimeRemaining = 0.f;
			ASC->GetGameplayEffectDurationAndTimeRemainingFromTag(CallbackTag, DurationTime, TimeRemaining);
			CooldownEndTime = ASC->GetWorld()->GetTimeSeconds() + TimeRemaining;

			bCanTick = true;
			OnCoolDownStarted();
		}
	}
	else if (NewCount == 0)
	{
		bCanTick = false;

		RemainingTime = 0.f;
		CooldownPercent = 0.f;

		OnCoolDownEnded();
	}
}
