// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjectiveConfig.h"

TSubclassOf<UAEQuestObjective> UQuestObjectiveConfig::GetQuestObjectiveClass() const
{
	return ObjectiveClass;
}
