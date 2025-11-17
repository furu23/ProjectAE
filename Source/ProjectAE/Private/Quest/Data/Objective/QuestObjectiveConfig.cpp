// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objective/QuestObjectiveConfig.h"

TSubclassOf<UAEQuestObjective> UQuestObjectiveConfig::GetQuestObjectiveClass() const
{
	// 이 함수는 반드시 서브클래스에서 구현되어야 합니다.
	ensureMsgf(false, TEXT("GetQuestObjectiveClass() is not implemented in %s"), *GetName());
	return nullptr;
}
