// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayEffectTypes.h"
#include "AEAbilitySystemComponent.h"
#include "AS_HealthSet.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthAttributeDataChanged, FOnAttributeChangeData);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMaxHealthAttributeDataChanged, FOnAttributeChangeData);

// Attribute 관리를 위한 함수 생성용 매크로
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class PROJECTAE_API UAS_HealthSet : public UAttributeSet
{
	GENERATED_BODY()

public:
    UAS_HealthSet();

    // **** 체력 변경에 대한 델리게이트를 제공합니다 ****
    
    // 최대 체력 변경에 대한 델리게이트입니다.
    FOnHealthAttributeDataChanged OnHealthChanged;

    // 최대 체력 변경에 대한 델리게이트입니다.
    FOnMaxHealthAttributeDataChanged OnMaxHealthChanged;

    // **** 속성 변경에 대한 상속 함수입니다 ****

    // 속성이 변경되기 전에 호출됩니다
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    // 속성이 변경된 후에 호출됩니다
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
    // GE가 속성 변경을 완료한 직후에 호출됩니다
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    // 전투 관련 속성입니다.

    UPROPERTY(BlueprintReadOnly, Category = "Attributes"/*, ReplicatedUsing = "OnRep_Health"*/)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UAS_HealthSet, Health);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes"/*, ReplicatedUsing = "OnRep_MaxHealth"*/)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UAS_HealthSet, MaxHealth);

protected:

    /*
    * 레플리케이션 함수입니다. 값 변경시, UI 적용 등에 사용해주세요.
    */

// 	UFUNCTION() virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
// 	UFUNCTION() virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
};
