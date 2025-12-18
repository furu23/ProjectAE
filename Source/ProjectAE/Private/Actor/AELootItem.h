// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AELootItem.generated.h"

class UBoxComponent;
class UGameplayEffect;

UCLASS()
class AAELootItem : public AActor
{
	GENERATED_BODY()

public:
	AAELootItem();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UBoxComponent> LootCollisionBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|GAS")
	TSubclassOf<UGameplayEffect> BioHealEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	float HealAmount = 10.f;
};