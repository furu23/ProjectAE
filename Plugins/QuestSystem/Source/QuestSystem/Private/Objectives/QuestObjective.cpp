// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/QuestObjective.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestManagerSubSystem.h"
#include "QuestSystem.h"

void UQuestObjective::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectQuestID)
{
	// 멤버 초기화
	ObjectiveConfig = Config;
	QuestID = ObjectQuestID;
	CachedQuestSys = QuestSys;
}

void UQuestObjective::Activate(UObject* WorldContext)
{
	if (IsComplete())
	{
		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective is completed"), *this->GetFName().ToString());

		bHasFiredCompletion = true;
		OnObjectiveCompleteDelegate.ExecuteIfBound(this);
	}
}
