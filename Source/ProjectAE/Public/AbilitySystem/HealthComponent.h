// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "HealthComponent.generated.h"

class UAbilitySystemComponent;
class UAS_HealthSet;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathSignature, AActor*, Causer, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamagedSiganature, AActor*, Causer, AActor*, Victim);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FOnDeathSignature OnDeathDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Health|Event")
	FOnDamagedSiganature OnDamageDelegate;

public:
	/**
	 * @brief ASC 와 AS를 찾아 캐싱해두는 (혹은 초기화하는) 함수입니다. 소유자 함수가 OnPossessed 함수, OnRep_PlayerState (혹은 액터 클래스)에서 호출해야 합니다.
	 * @return False : 초기화에 실패했으며. 반드시 필요한 사용에서는 check 혹은 ensure 디버그 도구를 활용할 것을 권장합니다
	 * @return True : 초기화에 성공했거나, 이미 소유자의 ASC 와 AS 를 캐싱해둔 상태입니다.
	 */
	virtual bool TryInitAbilitySystem(UAbilitySystemComponent* InASC);

	// **** 공용 API 함수 ****

	//virtual float GetHealth() const {}

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	UPROPERTY(Transient)
	TObjectPtr<const UAS_HealthSet> HealthSet;

	// **** 델리게이트 바인딩 함수 ****
	// 추후 GMS 도입 시에는 변경을 고려합니다.

	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);

	virtual void OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data);

private:
	// 사망 여부
	bool bIsDead = false;
};
