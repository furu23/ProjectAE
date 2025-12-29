// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "AEWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoAmountChangedSignature, int32, NewValue, int32, OldValue);

/**
* @brief 무기 관련 사항을 넣어둔 클래스입니다.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UAEWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    // 무기 교체 함수, 전략 패턴을 구현합니다.
    UFUNCTION(BlueprintCallable)
    void EquipWeapon(UAEWeaponDefinition* NewWeaponDef);

    // 탄약을 Max로 변경, 내부에서 델리게이트 호출
    UFUNCTION(BlueprintCallable)
    void ReloadWeapon();

    // 탄약을 Amount 만큼 소모, 내부에서 델리게이트 호출
    UFUNCTION(BlueprintCallable)
    bool UseAmmo(int32 AmmoUseAmount);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    int32 GetMaxAmmo() const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UPROPERTY(BlueprintAssignable)
    FOnAmmoAmountChangedSignature OnAmmoUseDelegate;

protected:
    // 현재 무기가 하나이므로 기본 무기를 주입받음
    UPROPERTY(/* , Replicated */)
    UAEWeaponDefinition* CurrentWeaponDef = nullptr;

private:
    int32 CurrentAmmo;

    // 현재 부여된 GA의 핸들 (교체 시 제거용)
    TArray<FGameplayAbilitySpecHandle> CurrentWeaponAbilityHandles;
};
