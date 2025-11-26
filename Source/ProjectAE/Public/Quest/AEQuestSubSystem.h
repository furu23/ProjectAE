// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestManagerSubSystem.h"
#include "AEQuestSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API UAEQuestSubSystem : public UQuestManagerSubSystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	virtual void OnQuestDataLoaded() override;
};
