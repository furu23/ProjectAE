// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/AEQuestSubSystem.h"

void UAEQuestSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAEQuestSubSystem::OnQuestDataLoaded()
{
	Super::OnQuestDataLoaded();

	StartActiveQuests();
}