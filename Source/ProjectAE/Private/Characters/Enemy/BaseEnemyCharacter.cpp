// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/BaseEnemyCharacter.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/AS_HealthSet.h"


UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	if (ASC)
	{
		return ASC;
	}
	return nullptr;
}

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	// ASC, BaseSet 
	ASC = CreateDefaultSubobject<UAEAbilitySystemComponent>("ASC");

	// AttributeSet µî·Ï
	ASC->AddAttributeSetSubobject<UAS_HealthSet>(HealthSet);
}
