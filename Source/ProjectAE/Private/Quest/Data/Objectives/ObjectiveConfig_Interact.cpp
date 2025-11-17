// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/ObjectiveConfig_Interact.h"
#include "Quest/Objectives/QuestObjective_Interact.h"

UObjectiveConfig_Interact::UObjectiveConfig_Interact()
{
	// 안전한 기본값 하드코딩 정의
	ObjectiveClass = UQuestObjective_Interact::StaticClass();
}
