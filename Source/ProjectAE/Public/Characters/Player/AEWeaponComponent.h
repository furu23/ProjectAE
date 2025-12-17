// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "AEWeaponComponent.generated.h"

/**
* @brief 무기 관련 사항을 넣어둔 클래스입니다.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UAEWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    // 현재 무기가 하나이므로 기본 무기를 주입받음
    UPROPERTY(/* , Replicated */)
    UAEWeaponDefinition* CurrentWeaponDef = nullptr;

    UPROPERTY(BlueprintReadWrite/* Replicated */)
    int32 CurrentAmmo;

    // 현재 부여된 GA의 핸들 (교체 시 제거용)
    TArray<FGameplayAbilitySpecHandle> CurrentWeaponAbilityHandles;

    // 무기 교체 함수, 전략 패턴을 구현합니다.
    UFUNCTION(BlueprintCallable)
    void EquipWeapon(UAEWeaponDefinition* NewWeaponDef);

    UFUNCTION(BlueprintCallable)
    int32 GetMaxAmmo();
};
