// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObjective_Kill.h"
#include "QuestSystem.h"
#include "QuestManagerSubSystem.h"
#include "ObjectiveConfig_Kill.h"

void UQuestObjective_Kill::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectiveQuestID)
{
	Super::Initialize(Config, QuestSys, ObjectiveQuestID);

	KillConfig = Cast<const UObjectiveConfig_Kill>(Config);
	checkf(KillConfig != nullptr, TEXT("ObjectiveConfig가 KillConfig 타입이 아닙니다!"));
}

void UQuestObjective_Kill::Activate(UObject* WorldContext)
{
	Super::Activate(WorldContext);
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective activating is started"), *this->GetFName().ToString());

	// 부모의 Activate에서 이미 목표가 완료되었는지 확인
	if (bHasFiredCompletion) return;

	if (!KillConfig || !WorldContext) return;

	// GMS를 가져옴
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContext);

	// 구독할 채널 가져옴
	FGameplayTag ListenTag = KillConfig->ListenTag;

	// 구독하고 반환하는 핸들 저장
	GMSListenHandle = GMS.RegisterListener<FQuestMessage_Generic, UQuestObjective_Kill>(
		ListenTag,
		this,
		&UQuestObjective_Kill::OnMessageReceived
	);

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective activating is listen [%s] now."), *this->GetFName().ToString(), *ListenTag.GetTagName().ToString());
	const TArray<TObjectPtr<UQuestTask>>& RefTask = KillConfig->TaskOnActivation;

	OnRequestTaskSignatureDelegate.ExecuteIfBound(RefTask);
}

void UQuestObjective_Kill::DeActivate()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective deactivating is successfully called"), *this->GetFName().ToString());

	if (GMSListenHandle.IsValid())
	{
		GMSListenHandle.Unregister();
	}
}

bool UQuestObjective_Kill::IsComplete() const
{
	if (!KillConfig) return false;
	
	FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);

	if (!ensure(ProgressData)) return false;

	return KillConfig->RequiredNumber <= ProgressData->ObjectProgress.FindRef(KillConfig->ObjectiveID);
}

void UQuestObjective_Kill::OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)
{
	FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] : [%s] objective faile to Get ProgressData."), *this->GetFName().ToString());
		return;
	}

	UE_LOG(LogQuestSystem, VeryVerbose, TEXT("[QuestSys] : [%s] objective is getting Message Now! \
		\nChecking Valid on Bool Property = %d,\
		\nChecking Valid on Reference Validating = %d,\
		\nChecking Listen Tag is Same = %d"),
		*this->GetFName().ToString(), bHasFiredCompletion, (!ProgressData || !Message.TargetActor), (Message.TargetTags.HasAll(KillConfig->TargetTags)));

	if (bHasFiredCompletion) return;

	if (!ProgressData || !Message.TargetActor) return;

	if (Message.TargetTags.HasAll(KillConfig->TargetTags))
	{
		ProgressData->ObjectProgress.FindOrAdd(KillConfig->ObjectiveID)++;

		if (IsComplete())
		{
			UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective is completed"), *this->GetFName().ToString());

			bHasFiredCompletion = true;
			OnObjectiveCompleteDelegate.ExecuteIfBound(this);
		}
	}
}
