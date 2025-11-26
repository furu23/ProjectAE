// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "GameplayTagContainer.h"
#include "AEQuestLocationActor.generated.h"

class UBoxComponent;

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTAE_API AAEQuestLocationActor : public AActor
{
	GENERATED_BODY()

public:
	AAEQuestLocationActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Component")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(EditDefaultsOnly, Category = "TargetTags")
	FGameplayTagContainer ObjectiveTags;

	bool bIsAlreadyTriggered = false;

};
