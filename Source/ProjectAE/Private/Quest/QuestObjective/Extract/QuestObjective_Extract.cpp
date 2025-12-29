// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestObjective_Extract.h"
#include "ObjectiveConfig_Extract.h"


UObjectiveConfig_Extract::UObjectiveConfig_Extract()
{
    // 안전한 기본값 하드코딩 정의
    ObjectiveClass = UQuestObjective_Extract::StaticClass();

    DescriptionFormat = NSLOCTEXT("Quest", "InteractDefault", "[진행 중] 탈출 0 / 1");
    CompletedDescription = NSLOCTEXT("Quest", "InteractComplete", "[완료] 탈출 완료");
}

FText UObjectiveConfig_Extract::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
    const int32 CurrentVal = ProgressData.ObjectProgress.FindRef(ObjectiveID);

    if (CurrentVal >= 1 && !CompletedDescription.IsEmpty())
    {
        return CompletedDescription;
    }

    FFormatNamedArguments Args;
    Args.Add(TEXT("InteractTarget"), FText::AsNumber(CurrentVal));

    return FText::Format(DescriptionFormat, Args);
}