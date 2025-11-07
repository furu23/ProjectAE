// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AS_BaseCombat.generated.h"

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
class PROJECTAE_API UAS_BaseCombat : public UAttributeSet
{
	GENERATED_BODY()

public:
    // 속성이 변경되기 전에 호출됩니다
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    // 속성이 변경된 후에 호출됩니다
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
    // GE가 속성 변경을 완료한 직후에 호출됩니다
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    // Replication 설정을 위한 필수 함수입니다.
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 전투 관련 속성입니다.

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_Health")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, Health);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_MaxHealth")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, MaxHealth);

    // 달리기 및 구르기 액션 관련 속성입니다.

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_Stamina")
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, Stamina);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_MaxStamina")
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, MaxStamina);

    // 탈출 강요 자원 관련 속성입니다.

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_Bio")
    FGameplayAttributeData Bio;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, Bio);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_MaxBio")
    FGameplayAttributeData MaxBio;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, MaxBio);

    // 기본 전투 관련 속성입니다.

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_BaseAttack")
    FGameplayAttributeData BaseAttack;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, BaseAttack);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = "OnRep_BaseDefense")
    FGameplayAttributeData BaseDefense;
    ATTRIBUTE_ACCESSORS(UAS_BaseCombat, BaseDefense);

protected:

    /*
    * 레플리케이션 함수입니다. 값 변경시, UI 적용 등에 사용해주세요.
    */

    UFUNCTION() virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
    UFUNCTION() virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
    UFUNCTION() virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
    UFUNCTION() virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
    UFUNCTION() virtual void OnRep_Bio(const FGameplayAttributeData& OldBio);
    UFUNCTION() virtual void OnRep_MaxBio(const FGameplayAttributeData& OldMaxBio);
    UFUNCTION() virtual void OnRep_BaseAttack(const FGameplayAttributeData& OldBaseAttack);
    UFUNCTION() virtual void OnRep_BaseDefense(const FGameplayAttributeData& OldBaseDefense);
};
