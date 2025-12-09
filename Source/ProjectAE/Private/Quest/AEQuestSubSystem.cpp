// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/AEQuestSubSystem.h"
#include "Core/GamePhaseSubsystem.h"

void UAEQuestSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 태스크 등록
	UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>(GetWorld());
	PhaseSys->RegisterLoadingTask(TEXT("QuestSystem"));

	// 초기화 시작
	StartAsyncLoadData();
}

void UAEQuestSubSystem::AcceptQuestForID(const FGameplayTag& QuestID)
{
	AcceptQuest(QuestID);
}

void UAEQuestSubSystem::CompleteQuestForID(const FGameplayTag& QuestID)
{
	ClaimQuestReward(QuestID);
}

void UAEQuestSubSystem::PreLoadGame(const TArray<uint8>& InData)
{
	this->LoadSaveData(InData);

	StartAsyncLoadData();
}

void UAEQuestSubSystem::OnQuestDataLoaded()
{
	Super::OnQuestDataLoaded();

	if (OnQuestEntryUpdatedDelegate.IsBound())
	{
		for (const auto& Pair : PlayerQuestHistory)
		{
			FQuestLogEntry Entry;
			if (BuildQuestLogEntry(Pair.Key, Entry))
			{
				UE_LOG(LogTemp, Log, TEXT("[QuestSys] OnQuestDataLoaded: Load [%s] ID Quest and Updating [%d] Progress Type"), *Pair.Key.ToString(), static_cast<int32>(Entry.CurrentState));
				OnQuestEntryUpdatedDelegate.Broadcast(Entry);
			}
		}
	}

	UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>(GetWorld());
	if (PhaseSys)
	{
		PhaseSys->CompleteLoadingTask(TEXT("QuestSystem"));
	}
}