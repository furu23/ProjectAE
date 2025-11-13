// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestManagerSubSystem.h"
#include "Core/AEGloabalHelper.h"

void UQuestManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UAEGloabalHelper::PrintString(FString("Hello SubSystem"));
}

void UQuestManagerSubSystem::OnPlayerLogin()
{

}

void UQuestManagerSubSystem::AcceptQuest()
{

}

void UQuestManagerSubSystem::ClaimQuestReward()
{

}

void UQuestManagerSubSystem::OnRaidStart()
{

}

void UQuestManagerSubSystem::OnRaidEnd()
{

}

void UQuestManagerSubSystem::LoadAndActivateQuest(FGameplayTag QuestID, FQuestProgressData* ProgressData)
{

}

void UQuestManagerSubSystem::DeactivateAndDestroyQuest(UAEQuestObject* QuestObject)
{

}