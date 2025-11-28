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

void UAEQuestSubSystem::OnQuestDataLoaded()
{
	Super::OnQuestDataLoaded();

	// 초기화 완료 즉시 퀘스트 활성화
	StartActiveQuests();

	// 태스크 완료
	UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>(GetWorld());
	PhaseSys->CompleteLoadingTask(TEXT("QuestSystem"));
}