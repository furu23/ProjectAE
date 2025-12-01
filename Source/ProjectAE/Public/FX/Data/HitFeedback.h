// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitFeedback.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FImpactFXInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UNiagaraSystem* VisualEffect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    USoundBase* SoundEffect = nullptr;
};

/**
 * 
 */
UCLASS()
class PROJECTAE_API UHitFeedback : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<TEnumAsByte<EPhysicalSurface>, FImpactFXInfo> SurfEffectMap;
	
};
