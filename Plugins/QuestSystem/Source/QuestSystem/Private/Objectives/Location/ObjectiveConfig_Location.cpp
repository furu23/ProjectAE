// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveConfig_Location.h"
#include "QuestObjective_Location.h"

UObjectiveConfig_Location::UObjectiveConfig_Location()
{
    // 안전한 기본값 하드코딩 정의
    ObjectiveClass = UQuestObjective_Location::StaticClass();

    DescriptionFormat = NSLOCTEXT("Quest", "LocationDefault", "[진행 중] 목표 장소 도달");
    CompletedDescription = NSLOCTEXT("Quest", "LocationComplete", "[완료] 확인 완료");
}

FText UObjectiveConfig_Location::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
    const int32 CurrentVal = ProgressData.ObjectProgress.FindRef(ObjectiveID);

    if (CurrentVal >= 1 && !CompletedDescription.IsEmpty())
    {
        return CompletedDescription;
    }

    return DescriptionFormat;
}