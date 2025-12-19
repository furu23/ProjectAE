// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AEPlayerState.generated.h"

class UAEAbilitySystemComponent;
class UAS_BaseCombat;
class UAS_HealthSet;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PROJECTAE_API AAEPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// ASC를 반환하는 공용 API 함수입니다.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	AAEPlayerState();

protected:

	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UAEAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UAS_BaseCombat> BaseSet;

	UPROPERTY()
	TObjectPtr<UAS_HealthSet> HealthSet;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayEffect> BioStarvationClass; 
};
