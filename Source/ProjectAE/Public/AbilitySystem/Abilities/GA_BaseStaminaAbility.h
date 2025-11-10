// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BaseStaminaAbility.generated.h"


class UGameplayEffect;
struct FOnAttributeChangeData;


/**
 * 
 */
UCLASS()
class PROJECTAE_API UGA_BaseStaminaAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// 어빌리티 사용 조건 함수 오버라이드입니다. 어빌리티 실행 전, 혹은 UI 등지에 사용합니다.
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags , const FGameplayTagContainer* TargetTags , OUT FGameplayTagContainer* OptionalRelevantTags) const override;
	
	// 어빌리티 실행 로직 함수 오버라이드입니다. 단발성/지속성 스테미나 소모를 모두 적용하며. 지속성의 경우 중간에 어빌리티를 취소하는 등의 델리게이트를 적용합니다.
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 스테미나 지속성 소모 종료를 위해 오버라이드합니다.
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// SetByCaller로 적용할 값
    UPROPERTY(EditDefaultsOnly, Category = "Cost|Stamina")
    float StaminaChunkCost = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Cost|Stamina", meta = (ToolTip = "단발성 스테미나 Cost GE를 배치합니다."))
	TSubclassOf<UGameplayEffect> StaminaChunkCostGE;

	// SetByCaller로 적용할 값
    UPROPERTY(EditDefaultsOnly, Category = "Cost|Stamina")
    float StaminaDrainPerSecond = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Cost|Stamina", meta = (ToolTip = "지속성 스테미나 Cost GE를 배치합니다."))
	TSubclassOf<UGameplayEffect> StaminaDrainCostGE;

	// 지속성 GE 핸들 (GA_Sprint용)
	FActiveGameplayEffectHandle DrainEffectHandle;

	// 스태미나 0일 때 자동 취소용 델리게이트 핸들
	FDelegateHandle StaminaChangedDelegateHandle;

	virtual void OnStaminaChanged(const FOnAttributeChangeData& Data);
};
