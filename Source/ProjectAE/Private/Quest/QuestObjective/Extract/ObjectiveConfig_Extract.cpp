// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObjective_Extract.h"
#include "ObjectiveConfig_Extract.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestManagerSubSystem.h"
#include "QuestTypes.h"

void UQuestObjective_Extract::Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectiveQuestID)
{
	Super::Initialize(Config, QuestSys, ObjectiveQuestID);

	ExtractConfig = Cast<const UObjectiveConfig_Extract>(Config);
	checkf(ExtractConfig != nullptr, TEXT("ObjectiveConfig가 ExtractConfig 타입이 아닙니다!"));
}

void UQuestObjective_Extract::Activate(UObject* WorldContext)
{
	UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective activating is started"), *this->GetFName().ToString());
	Super::Activate(WorldContext);

	// 부모의 Activate에서 이미 목표가 완료되었는지 확인
	if (bHasFiredCompletion) return;

	if (!ExtractConfig || !WorldContext) return;

	// GMS를 가져옴
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContext);

	// 구독할 채널 가져옴
	FGameplayTag ListenTag = ExtractConfig->ListenTag;

	// 구독하고 반환하는 핸들 저장
	GMSListenHandle = GMS.RegisterListener<FQuestMessage_Generic, UQuestObjective_Extract>(
		ListenTag,
		this,
		&UQuestObjective_Extract::OnMessageReceived
	);

	UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective activating is listen [%s] now."), *this->GetFName().ToString(), *ListenTag.GetTagName().ToString());
	const TArray<TObjectPtr<UQuestTask>>& RefTask = ExtractConfig->TaskOnActivation;

	OnRequestTaskSignatureDelegate.ExecuteIfBound(RefTask);
}

void UQuestObjective_Extract::DeActivate()
{
	UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective deactivating is successfully called"), *this->GetFName().ToString());

	if (GMSListenHandle.IsValid())
	{
		GMSListenHandle.Unregister();
	}
}

bool UQuestObjective_Extract::IsComplete() const
{
	const FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestSys] : [%s] objective faile to Get ProgressData."), *this->GetFName().ToString());
		return false;
	}

	if (!ExtractConfig) return false;

	const int32 ProgressPtr = ProgressData->ObjectProgress.FindRef(ExtractConfig->ObjectiveID);
	if (ProgressPtr)
	{
		return ProgressPtr >= 1;
	}
	return false;
}

void UQuestObjective_Extract::OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)
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
		*this->GetFName().ToString(), bHasFiredCompletion, (!ProgressData || !Message.TargetActor), (Message.TargetTags.HasAll(ExtractConfig->TargetTags)));

	if (bHasFiredCompletion) return;

	if (!ProgressData || !Message.TargetActor) return;

	if (Message.TargetTags.HasAll(ExtractConfig->TargetTags))
	{
		ProgressData->ObjectProgress.FindOrAdd(ExtractConfig->ObjectiveID)++;
		if (IsComplete())
		{
			UE_LOG(LogTemp, Verbose, TEXT("[QuestSys] : [%s] objective is completed"), *this->GetFName().ToString());

			bHasFiredCompletion = true;
			OnObjectiveCompleteDelegate.ExecuteIfBound(this);
		}
	}
}
