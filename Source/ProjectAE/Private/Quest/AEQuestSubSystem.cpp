// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/AEQuestSubSystem.h"

void UAEQuestSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAEQuestSubSystem::AcceptQuestForID(const FGameplayTag& QuestID)
{
	AcceptQuest(QuestID);
}

void UAEQuestSubSystem::CompleteQuestForID(const FGameplayTag& QuestID)
{
	ClaimQuestReward(QuestID);
}

void UAEQuestSubSystem::OnQuestDataLoaded()
{
	Super::OnQuestDataLoaded();

	StartActiveQuests();
}