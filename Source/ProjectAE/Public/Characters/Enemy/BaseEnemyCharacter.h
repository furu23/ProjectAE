// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemInterface.h"
#include "BaseEnemyCharacter.generated.h"

class UAEAbilitySystemComponent;
class UAS_HealthSet;

/**
 * 
 */
UCLASS()
class PROJECTAE_API ABaseEnemyCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// ASC를 반환하는 공용 API 함수입니다.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	ABaseEnemyCharacter();

protected:
	UPROPERTY()
	TObjectPtr<UAEAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UAS_HealthSet> HealthSet;
};
