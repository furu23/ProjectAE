// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/QuestObjective.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void UQuestObjective::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectQuestID)
{
	// 멤버 초기화
	ObjectiveConfig = Config;
	QuestID = ObjectQuestID;
	CachedQuestSys = QuestSys;
}
