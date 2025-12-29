// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HealthComponent.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AS_HealthSet.h"
#include "QuestMessageHelpers.h"
#include "GameplayEffectExtension.h"
#include "../../../../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayEffect.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

bool UHealthComponent::TryInitAbilitySystem(UAbilitySystemComponent* InASC)
{
	if (CachedASC)
	{
		return true;
	}

	CachedASC = InASC;

	if (CachedASC)
	{
		 HealthSet = CachedASC->GetSet<UAS_HealthSet>();
		 if (HealthSet)
		 {
			CachedASC->GetGameplayAttributeValueChangeDelegate(UAS_HealthSet::GetHealthAttribute()).AddUObject(this, &UHealthComponent::OnHealthAttributeChanged);
			CachedASC->GetGameplayAttributeValueChangeDelegate(UAS_HealthSet::GetMaxHealthAttribute()).AddUObject(this, &UHealthComponent::OnMaxHealthAttributeChanged);

			if (DefineEffectClass)
			{
				FGameplayEffectContextHandle EffectHandle = InASC->MakeEffectContext();
				EffectHandle.AddSourceObject(this);
				EffectHandle.AddInstigator(GetOwner(), GetOwner());

				FGameplayEffectSpecHandle Spec = InASC->MakeOutgoingSpec(DefineEffectClass, 1, EffectHandle);
				if (!ensureMsgf(Spec.IsValid(), TEXT("Critical Error For MakeOutGoingSpec On HealthComp."))) return false;

				Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Health")), InitialHealthValue);

				InASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, InASC);
			}

			return true;
		 }
	}
	UE_LOG(LogTemp, Error, TEXT("Failed To Access ASC OR AttributeSet In HealthComponent. Initializing is decompolete"));
	return false;
}

void UHealthComponent::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 이미 죽었다면 리턴
	if (bIsDead)
	{
		return;
	}

	// 데미지 하향 변동 시
	if (Data.OldValue > Data.NewValue)
	{
		// 대상 찾아보기
		AActor* Instigator = nullptr;
		if (Data.GEModData != nullptr)
		{
			const FGameplayEffectContextHandle& Context = Data.GEModData->EffectSpec.GetContext();
			Instigator = Context.GetInstigator();
		}

		// 사망 시에
		if (Data.NewValue <= 0.f)
		{
			OnDeathDelegate.Broadcast(Instigator, GetOwner());

			// 퀘스트 목표 갱신을 위해 사망 GMS를 방송
			if (ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner()))
			{
				if (OwnerCharacter->GetCharacterTag().IsValid())
				{
					// QuestSystem의 헬퍼 함수 호출
					UQuestMessageHelpers::BroadcastAIKilledEvent(
						this,
						Instigator,
						OwnerCharacter,
						OwnerCharacter->GetCharacterTag()
					);
				}
			}
		}
		// 데미지 받았을 시에.
		else
		{
			OnDamageDelegate.Broadcast(Instigator, GetOwner());
		}
	}
}

void UHealthComponent::OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 여기서 최대 체력 변화요소를 정리합니다.
}
