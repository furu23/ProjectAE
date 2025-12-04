// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/SkillSlotWidget.h"
#include "AbilitySystemComponent.h"

void USkillSlotWidget::BindToASC(UAbilitySystemComponent* ASC, const FGameplayTag& InCoolDownTag)
{
	WeakASC = ASC;
	CoolDownTag = InCoolDownTag;

	if (WeakASC.IsValid() && CoolDownTag.IsValid())
	{
		ASC->RegisterGameplayTagEvent(CoolDownTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &USkillSlotWidget::OnCooldownTagChanged);
	}
	
}

void USkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bCanTick) { return; }


}

void USkillSlotWidget::OnCooldownTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		UAbilitySystemComponent* ASC = WeakASC.Get();

		if (ASC)
		{
			float TimeRemaining = 0.f;
			ASC->GetGameplayEffectDurationAndTimeRemainingFromTag(CallbackTag, DurationTime, TimeRemaining);
			ColdownStartTime = ASC->GetWorld()->GetTimeSeconds() + (DurationTime - TimeRemaining);

			bCanTick = true;
		}
	}
}
