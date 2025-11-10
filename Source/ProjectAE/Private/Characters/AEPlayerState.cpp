// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AEPlayerState.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/AS_BaseCombat.h"

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
	ASC = CreateDefaultSubobject<UAEAbilitySystemComponent>("ASC");
	AS = CreateDefaultSubobject<UAS_BaseCombat>("AttributeSet");
	ASC->AddAttributeSetSubobject<UAS_BaseCombat>(AS);
}
