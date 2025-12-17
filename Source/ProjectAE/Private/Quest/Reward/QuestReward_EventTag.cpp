// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Reward/QuestReward_EventTag.h"
#include "Core/SaveGameSubsystem.h"
#include "Core/AEGlobalHelper.h"

void UQuestReward_EventTag::GiveReward(UObject* WorldContextObject)
{
	USaveGameSubsystem* SaveSys = UAEGlobalHelper::GetSaveGameSubsystem(WorldContextObject);
	if (!SaveSys) return;

	if (!SaveSys->IsEventCompleted(EventTag))
	{
		SaveSys->MarkEventCompleted(EventTag);
	}
}

FString UQuestReward_EventTag::GetDescription() const
{
	return Super::GetDescription();
}
