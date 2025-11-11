// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HealthComponent.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AS_HealthSet.h"

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
			return true;
		 }
	}
	UE_LOG(LogTemp, Error, TEXT("Failed To Access ASC OR AttributeSet In HealthComponent. Initializing is decompolete"));
	return false;
}

void UHealthComponent::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 여기서 체력 변화요소를 정리합니다.
}

void UHealthComponent::OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 여기서 최대 체력 변화요소를 정리합니다.
}
