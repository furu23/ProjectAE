// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/Location/ObjectiveConfig_Location.h"
#include "QuestObjective_Location.h"

UObjectiveConfig_Location::UObjectiveConfig_Location()
{
    // 안전한 기본값 하드코딩 정의
    ObjectiveClass = UQuestObjective_Location::StaticClass();
}

FText UObjectiveConfig_Location::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
    const int32* ProgressPtr = ProgressData.ObjectProgress.Find(ObjectiveID);
    int32 CurrentProgress = ProgressPtr ? *ProgressPtr : 0;

    return FText();
}