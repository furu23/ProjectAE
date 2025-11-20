// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AEWeaponDefinition.generated.h"

class UGameplayAbility;
struct FInputAbilitySet;

/**
 * @brief 무기 (총)의 기본 데이터를 저장해둔 데이터 에셋입니다.
 * @note 이 데이터 에셋을 가진 컴포넌트가 GA를 부여 및 해제합니다.
 * 
 * @see UAECombatComponent
 */
UCLASS()
class PROJECTAE_API UAEWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 부여할 게임플레이 어빌리티를 전략 패턴으로 구현합니다.
    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    TArray<TSubclassOf<UGameplayAbility>> WeaponAbilities;

    // 2. Stats (데이터)
    UPROPERTY(EditDefaultsOnly, Category = "Stats")
    int32 MaxAmmo;
    
    UPROPERTY(EditDefaultsOnly, Category = "Stats")
    float BaseDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Stats")
    UCurveFloat* RecoilCurve;

    UPROPERTY(EditDefaultsOnly, Category = "FX")
    UParticleSystem* MuzzleFlash;

    UPROPERTY(EditDefaultsOnly, Category = "FX")
    USoundBase* FireSound;

	// 추후 필요시 구현 (아이템 태그로 AssetName, 타입은 새로 지정해주는 것을 권장)
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
