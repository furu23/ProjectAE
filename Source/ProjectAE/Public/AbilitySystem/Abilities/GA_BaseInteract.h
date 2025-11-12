// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BaseInteract.generated.h"

/**
 * 상호작용을 위해 TSubClassOf 의 형태로 InteractableActor의 필드로 존재합니다.
 * InteractComponent에서 이 Ability를 Pull 하는 방식으로 Owner Character의 ASC에 이 어빌리티를 부여합니다.
 * TargetActor의 실구현, 및 기반 로직을 이 기본 클래스에서 작성합니다.
 */
UCLASS()
class PROJECTAE_API UGA_BaseInteract : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetTargetActorFromSpec() const;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /* = nullptr */, const FGameplayTagContainer* TargetTags /* = nullptr */, OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const override;

	UPROPERTY(BlueprintReadOnly, Category = "interaction")
	TObjectPtr<AActor> CachedTargetActor;
};
