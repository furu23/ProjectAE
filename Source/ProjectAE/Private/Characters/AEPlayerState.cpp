// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AEPlayerState.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/AS_BaseCombat.h"
#include "AbilitySystem/AS_HealthSet.h"

UAbilitySystemComponent* AAEPlayerState::GetAbilitySystemComponent() const
{
	if (ASC)
	{
		return ASC;
	}
	return nullptr;
}

AAEPlayerState::AAEPlayerState()
{
	// ASC, BaseSet 
	ASC = CreateDefaultSubobject<UAEAbilitySystemComponent>("ASC");
	BaseSet = CreateDefaultSubobject<UAS_BaseCombat>("AttributeSet");
	HealthSet = CreateDefaultSubobject<UAS_HealthSet>("HealthSet");


	// AttributeSet 등록
	ASC->AddAttributeSetSubobject<UAS_BaseCombat>(BaseSet);
	ASC->AddAttributeSetSubobject<UAS_HealthSet>(HealthSet);
}

void AAEPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (BioStarvationClass)
	{
		BaseSet->BioStarvationClass = BioStarvationClass;
	}
}
