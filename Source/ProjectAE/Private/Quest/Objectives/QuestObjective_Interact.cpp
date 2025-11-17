// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Objectives/QuestObjective_Interact.h"
#include "Quest/Data/Objectives/ObjectiveConfig_Interact.h"
#include "Quest/Data/Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Quest/AEQuestTypes.h"

void UQuestObjective_Interact::Initialize(const UQuestObjectiveConfig* Config, FQuestProgressData* ProgressRef)
{
	Super::Initialize(Config, ProgressRef);

	InteractConfig = Cast<const UObjectiveConfig_Interact>(Config);
	checkf(InteractConfig != nullptr, TEXT("ObjectiveConfig가 InteractConfig 타입이 아닙니다!"));
}

void UQuestObjective_Interact::Activate(UObject* WorldContext)
{
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

	// OnRequestTaskSignatureDelegate.ExecuteIfBound(InteractConfig->TaskOnActivation());
}

void UQuestObjective_Interact::DeActivate()
{
	GMSListenHandle.Unregister();
}

bool UQuestObjective_Interact::IsComplete() const
{
	if (!InteractConfig || !ProgressDataRef) return false;

	const int32* ProgressPtr = ProgressDataRef->ObjectProgress.Find(InteractConfig->ObjectiveID);
	if (ProgressPtr)
	{
		return *ProgressPtr >= 1;
	}
	return false;
}

void UQuestObjective_Interact::OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)
{
	if (bHasFiredCompletion) return;

	if (!ProgressDataRef || !Message.TargetActor) return;

	if (Message.TargetTags.HasAll(InteractConfig->TargetTags))
	{
		ProgressDataRef->ObjectProgress.FindOrAdd(InteractConfig->ObjectiveID)++;
		if (IsComplete())
		{
			OnObjectiveCompleteDelegate.ExecuteIfBound(this);

			DeActivate();
			bHasFiredCompletion = true;
		}
	}
}
