// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpawnEnemyData.generated.h"

class ABaseEnemyCharacter;

/**
 * 
 */
UCLASS()
class PROJECTAE_API USpawnEnemyData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	TSubclassOf<ABaseEnemyCharacter> EnemyClassToSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	float SpawnTime = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	float InitialHealth = 100.f;
	
};
