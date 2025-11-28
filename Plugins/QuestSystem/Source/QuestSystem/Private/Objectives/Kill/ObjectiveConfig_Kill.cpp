// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveConfig_Kill.h"

UObjectiveConfig_Kill::UObjectiveConfig_Kill()
{
	// 안전한 기본값 하드코딩 정의
	ObjectiveClass = UObjectiveConfig_Kill::StaticClass();

	DescriptionFormat = NSLOCTEXT("Quest", "KillDefault", "[진행 중] 적 {Current} / {Target} 처치");
	CompletedDescription = NSLOCTEXT("Quest", "KillComplete", "[완료] 적 처치 완료");
}

FText UObjectiveConfig_Kill::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
    const int32 CurrentVal = ProgressData.ObjectProgress.FindRef(ObjectiveID);

    if (CurrentVal >= RequiredNumber && !CompletedDescription.IsEmpty())
    {
        return CompletedDescription;
    }

    FFormatNamedArguments Args;
    Args.Add(TEXT("Current"), FText::AsNumber(CurrentVal));
    Args.Add(TEXT("Target"), FText::AsNumber(RequiredNumber));

    return FText::Format(DescriptionFormat, Args);
}
