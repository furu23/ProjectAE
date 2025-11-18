// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/Config/QuestObjectiveConfig.h"

TSubclassOf<UQuestObjective> UQuestObjectiveConfig::GetQuestObjectiveClass() const
{
	return ObjectiveClass;
}

FText UQuestObjectiveConfig::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
	return FText::FromString(TEXT("Invalid Objective"));
}
