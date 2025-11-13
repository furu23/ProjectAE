// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestObjectiveConfig.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECTAE_API UQuestObjectiveConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToopTip = "FQuestProgressData 의 TMap 에 Key 값이 됩니다."))
	FGameplayTag ObjectiveID;

	// 이 퀘스트가 사용할 런타임 객체, QuestObjective를 반환합니다. ex) QuestObjective_Kill::StaticClass();
	// virtual TSubclassOf<AEQuestObjective> GetQuestObjectiveClass() const;
};
