// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObjective_Deliver.h"
#include "QuestManagerSubSystem.h"
#include "ObjectiveConfig_Deliver.h"

void UQuestObjective_Deliver::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectiveQuestID)
{
	Super::Initialize(Config, QuestSys, ObjectiveQuestID);

	DeliverConfig = Cast<const UObjectiveConfig_Deliver>(Config);
	checkf(DeliverConfig != nullptr, TEXT("ObjectiveConfig가 DeliverConfig 타입이 아닙니다!"));
}

void UQuestObjective_Deliver::Activate(UObject* WorldContext)
{
	Super::Activate(WorldContext);
	UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective activating is started"), *this->GetFName().ToString());

	// 부모의 Activate에서 이미 목표가 완료되었는지 확인
	if (bHasFiredCompletion) return;

	if (!DeliverConfig || !WorldContext) return;

	// GMS를 가져옴
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContext);

	// 구독할 채널 가져옴
	FGameplayTag ListenTag = DeliverConfig->ListenTag;

	// 구독하고 반환하는 핸들 저장
	GMSListenHandle = GMS.RegisterListener<FQuestMessage_Generic, UQuestObjective_Deliver>(
		ListenTag,
		this,
		&UQuestObjective_Deliver::OnMessageReceived
	);

	UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective activating is listen [%s] now."), *this->GetFName().ToString(), *ListenTag.GetTagName().ToString());
	const TArray<TObjectPtr<UQuestTask>>& RefTask = DeliverConfig->TaskOnActivation;

	OnRequestTaskSignatureDelegate.ExecuteIfBound(RefTask);
}

void UQuestObjective_Deliver::DeActivate()
{
	UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective deactivating is successfully called"), *this->GetFName().ToString());

	if (GMSListenHandle.IsValid())
	{
		GMSListenHandle.Unregister();
	}
}

bool UQuestObjective_Deliver::IsComplete() const
{
	if (!DeliverConfig) return false;

	FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);

	if (!ensure(ProgressData)) return false;

	const int32 ProgressVal = ProgressData->ObjectProgress.FindRef(DeliverConfig->ObjectiveID);

	return ProgressVal >= 1;
}

void UQuestObjective_Deliver::OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)
{
	FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestSys] : [%s] objective faile to Get ProgressData."), *this->GetFName().ToString());
		return;
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("[QuestSys] : [%s] objective is getting Message Now! \
		\nChecking Valid on Bool Property = %d,\
		\nChecking Valid on Reference Validating = %d,\
		\nChecking Listen Tag is Same = %d"),
		*this->GetFName().ToString(), bHasFiredCompletion, (!ProgressData || !Message.TargetActor), (Message.TargetTags.HasTag(DeliverConfig->TargetEventTag)));

	if (bHasFiredCompletion) return;

	if (!ProgressData || !Message.TargetActor) return;

	if (Message.TargetTags.HasTag(DeliverConfig->TargetEventTag))
	{
		ProgressData->ObjectProgress.FindOrAdd(DeliverConfig->ObjectiveID)++;

		if (IsComplete())
		{
			UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective is completed"), *this->GetFName().ToString());

			bHasFiredCompletion = true;
			OnObjectiveCompleteDelegate.ExecuteIfBound(this);
		}
	}
}
