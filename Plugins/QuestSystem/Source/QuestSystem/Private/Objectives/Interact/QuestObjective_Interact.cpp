// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObjective_Interact.h"
#include "ObjectiveConfig_Interact.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestManagerSubSystem.h"
#include "QuestTypes.h"
#include "QuestSystem.h"

void UQuestObjective_Interact::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectQuestID)
{
	Super::Initialize(Config, QuestSys, ObjectQuestID);

	InteractConfig = Cast<const UObjectiveConfig_Interact>(Config);
	checkf(InteractConfig != nullptr, TEXT("ObjectiveConfig가 InteractConfig 타입이 아닙니다!"));
}

void UQuestObjective_Interact::Activate(UObject* WorldContext)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective activating is started"), *this->GetFName().ToString());
	Super::Activate(WorldContext);

	// 부모의 Activate에서 이미 목표가 완료되었는지 확인
	if (bHasFiredCompletion) return;

	if (!InteractConfig || !WorldContext) return;

	// GMS를 가져옴
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContext);
	
	// 구독할 채널 가져옴
	FGameplayTag ListenTag = InteractConfig->ListenTag;

	// 구독하고 반환하는 핸들 저장
	GMSListenHandle = GMS.RegisterListener<FQuestMessage_Generic, UQuestObjective_Interact>(
		ListenTag,
		this,
		&UQuestObjective_Interact::OnMessageReceived
	);

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective activating is listen [%s] now."), *this->GetFName().ToString(), *ListenTag.GetTagName().ToString());
	const TArray<TObjectPtr<UQuestTask>>& RefTask = InteractConfig->TaskOnActivation;

	OnRequestTaskSignatureDelegate.ExecuteIfBound(RefTask);
}

void UQuestObjective_Interact::DeActivate()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective deactivating is successfully called"), *this->GetFName().ToString());

	if (GMSListenHandle.IsValid())
	{
		GMSListenHandle.Unregister();
	}
}

bool UQuestObjective_Interact::IsComplete() const
{
	const FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] : [%s] objective faile to Get ProgressData."), *this->GetFName().ToString());
		return false;
	}
	
	if (!InteractConfig) return false;

	const int32 ProgressPtr = ProgressData->ObjectProgress.FindRef(InteractConfig->ObjectiveID);
	if (ProgressPtr)
	{
		return ProgressPtr >= 1;
	}
	return false;
}

void UQuestObjective_Interact::OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)
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
		*this->GetFName().ToString(), bHasFiredCompletion, (!ProgressData || !Message.TargetActor), (Message.TargetTags.HasAll(InteractConfig->TargetTags)));

	if (bHasFiredCompletion) return;

	if (!ProgressData || !Message.TargetActor) return;

	if (Message.TargetTags.HasAll(InteractConfig->TargetTags))
	{
		ProgressData->ObjectProgress.FindOrAdd(InteractConfig->ObjectiveID)++;
		if (IsComplete())
		{
			UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] objective is completed"), *this->GetFName().ToString());

			bHasFiredCompletion = true;
			OnObjectiveCompleteDelegate.ExecuteIfBound(this);
		}
	}
}
