// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveConfig_Deliver.h"
#include "QuestObjective_Deliver.h"

UObjectiveConfig_Deliver::UObjectiveConfig_Deliver()
{
    // 안전한 기본값 하드코딩 정의
    ObjectiveClass = UQuestObjective_Deliver::StaticClass();

    DescriptionFormat = NSLOCTEXT("Quest", "DeliverDefault", "[진행 중] 아이템 제출 0 / 1 (bool 활성화 시: [진행 중] {Item} {Amount} 제출 {Current} / 1)");
    CompletedDescription = NSLOCTEXT("Quest", "DeliverComplete", "[완료] 제출 완료");
}

FText UObjectiveConfig_Deliver::GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const
{
    const int32 CurrentVal = ProgressData.ObjectProgress.FindRef(ObjectiveID);

    if (CurrentVal >= 1 && !CompletedDescription.IsEmpty())
    {
        return CompletedDescription;
    }

    FFormatNamedArguments Args;

    Args.Add(TEXT("Current"), FText::AsNumber(CurrentVal));

    if (bBroadcastRequirementInfo)
    {
        Args.Add(TEXT("Item"), FText::FromName(ItemID)); // FName을 FText로 변환
        Args.Add(TEXT("Amount"), FText::AsNumber(ItemAmount));
    }
    else
    {
        Args.Add(TEXT("Item"), FText::FromString(TEXT("Unknown Item")));
        Args.Add(TEXT("Amount"), FText::FromString(TEXT("?")));
    }

    return FText::Format(DescriptionFormat, Args);
}

bool UObjectiveConfig_Deliver::GetRequirementInfo(FGameplayTag& OutObjectiveTag, FName& OutItemID, int32& OutItemAmount) const
{
    if (!bBroadcastRequirementInfo || ItemID == NAME_None || ItemAmount == -1)
    {
        ensureMsgf(false, TEXT("GetRequirementInfo is called, but Invalid Accessing Here. careful with call timing or property detail."));
        OutObjectiveTag = FGameplayTag::EmptyTag;
        OutItemID = NAME_None;
        OutItemAmount = -1;
        return false;
    }

    OutObjectiveTag = ObjectiveID;
    OutItemID = ItemID;
    OutItemAmount = ItemAmount;
    return true;
}
