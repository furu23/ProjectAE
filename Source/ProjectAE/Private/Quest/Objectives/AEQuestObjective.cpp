// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Objectives/AEQuestObjective.h"
#include "Quest/Data/Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void UAEQuestObjective::Initialize(const UQuestObjectiveConfig* Config, FQuestProgressData* ProgressRef)
{
	// 멤버 초기화
	ObjectiveConfig = Config;
	ProgressDataRef = ProgressRef;
}
