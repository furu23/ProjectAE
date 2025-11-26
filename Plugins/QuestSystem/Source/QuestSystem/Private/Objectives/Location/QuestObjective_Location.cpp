// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/Location/QuestObjective_Location.h"
#include "Objectives/Location/ObjectiveConfig_Location.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestManagerSubSystem.h"
#include "QuestTypes.h"
#include "QuestSystem.h"

void UQuestObjective_Location::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectQuestID)
{
	Super::Initialize(Config, QuestSys, ObjectQuestID);

	InteractConfig = Cast<const UObjectiveConfig_Location>(Config);
	checkf(InteractConfig != nullptr, TEXT("ObjectiveConfig가 InteractConfig 타입이 아닙니다!"));
}

void UQuestObjective_Location::Activate(UObject* WorldContext)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] objective activating is started"), *this->GetFName().ToString());

	if (!InteractConfig || !WorldContext) return;

	// GMS를 가져옴
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContext);

	// 구독할 채널 가져옴
	FGameplayTag ListenTag = InteractConfig->ListenTag;

	// 구독하고 반환하는 핸들 저장
	GMSListenHandle = GMS.RegisterListener<FQuestMessage_Generic, UQuestObjective_Location>(
		ListenTag,
		this,
		&UQuestObjective_Location::OnMessageReceived
	);

	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] objective activating is listen [%s] now."), *this->GetFName().ToString(), *ListenTag.GetTagName().ToString());
	// OnRequestTaskSignatureDelegate.ExecuteIfBound(InteractConfig->TaskOnActivation());
}

void UQuestObjective_Location::DeActivate()
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] objective deactivating is successfully called"), *this->GetFName().ToString());

	GMSListenHandle.Unregister();
}

bool UQuestObjective_Location::IsComplete() const
{
	const FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] : [%s] objective faile to Get ProgressData."), *this->GetFName().ToString());
		return false;
	}

	if (!InteractConfig || !ProgressData) return false;

	const int32 ProgressVal = ProgressData->ObjectProgress.FindRef(InteractConfig->ObjectiveID);
	if (ProgressVal)
	{
		return ProgressVal >= 1;
	}
	return false;
}

void UQuestObjective_Location::OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)
{
	FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] : [%s] objective faile to Get ProgressData."), *this->GetFName().ToString());
		return;
	}
	
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] objective is getting Message Now! \
		\nChecking Valid on Bool Property = %d,\
		\nChecking Valid on Reference Validating = %d,\
		\nChecking Listen Tag is Same = %d"),
		*this->GetFName().ToString(), bHasFiredCompletion, (!ProgressData || !Message.TargetActor), (Message.TargetTags.HasAll(InteractConfig->TargetTags)));

	if (bHasFiredCompletion) return;

	if (!Message.TargetActor) return;

	if (Message.TargetTags.HasAll(InteractConfig->TargetTags))
	{
		ProgressData->ObjectProgress.FindOrAdd(InteractConfig->ObjectiveID)++;
		if (IsComplete())
		{
			UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] objective is completed"), *this->GetFName().ToString());

			bHasFiredCompletion = true;
			OnObjectiveCompleteDelegate.ExecuteIfBound(this);
		}
	}
}
