// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveConfig_Interact.h"
#include "QuestObjective_Interact.h"

UObjectiveConfig_Interact::UObjectiveConfig_Interact()
{
	// 안전한 기본값 하드코딩 정의
	ObjectiveClass = UQuestObjective_Interact::StaticClass();

    DescriptionFormat = NSLOCTEXT("Quest", "InteractDefault", "[진행 중] 상호작용 대상 0 / 1");
    CompletedDescription = NSLOCTEXT("Quest", "InteractComplete", "[완료] 상호작용 완료");
}

FText UObjectiveConfig_Interact::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
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