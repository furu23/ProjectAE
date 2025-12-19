// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Component")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(EditInstanceOnly, Category = "TargetTags")
	FGameplayTag QuestEventTag;
};
