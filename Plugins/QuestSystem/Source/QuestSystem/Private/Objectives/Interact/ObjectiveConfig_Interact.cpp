// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/Interact/ObjectiveConfig_Interact.h"
#include "Objectives/Interact/QuestObjective_Interact.h"

UObjectiveConfig_Interact::UObjectiveConfig_Interact()
{
	// 안전한 기본값 하드코딩 정의
	ObjectiveClass = UQuestObjective_Interact::StaticClass();
}

FText UObjectiveConfig_Interact::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
    const int32* ProgressPtr = ProgressData.ObjectProgress.Find(ObjectiveID);
    int32 CurrentProgress = ProgressPtr ? *ProgressPtr : 0;

    FString Status = (CurrentProgress >= 1) ? TEXT("[완료]") : TEXT("[미완료]");

    return FText::Format(
        FText::FromString(TEXT("{0}")),
        FText::FromString(Status)
    );
}