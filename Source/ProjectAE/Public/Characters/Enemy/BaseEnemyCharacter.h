// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemInterface.h"
#include "BaseEnemyCharacter.generated.h"

class UAEAbilitySystemComponent;
class UAS_HealthSet;
class UHealthComponent;

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
	virtual void BeginPlay();

	virtual void PossessedBy(AController* NewController);

	// 캐릭터가 죽었을 때 호출될 함수
	UFUNCTION(BlueprintNativeEvent, Category = "Character|Event")
	void OnDeath(AActor* Causer, AActor* Victim);

	// ASC
	TObjectPtr<UAEAbilitySystemComponent> ASC;

	// 체력 컴포넌트
	TObjectPtr<UHealthComponent> HealthComp;

	// 체력 어트리뷰트
	TObjectPtr<UAS_HealthSet> HealthSet;
};
