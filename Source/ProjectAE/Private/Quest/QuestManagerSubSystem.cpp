// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestManagerSubSystem.h"
#include "Core/AEGloabalHelper.h"
#include "Quest/AEQuestTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Quest/Data/DA_QuestBase.h"
#include "Quest/AEQuestObject.h"

void UQuestManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

TArray<FQuestLogEntry> UQuestManagerSubSystem::GetQuestLogEntries() const
{
	TArray<FQuestLogEntry> LogEntries;
	for (const TPair<FGameplayTag, FQuestProgressData>& Pair : PlayerQuestHistory)
	{
		FQuestLogEntry Entry;
		if (BuildQuestLogEntry(Pair.Key, Pair.Value, Entry))
		{
			LogEntries.Add(Entry);
		}
	}
	return LogEntries;
}

void UQuestManagerSubSystem::NotifyQuestUpdate(FGameplayTag QuestID)
{
	// 델리게이트에 구독자가 있는지 확인
	if (OnQuestEntryUpdated.IsBound())
	{
		// DTO 빌더를 호출하여 최신 DTO 생성
		FQuestLogEntry UpdatedEntry;
		if (BuildQuestLogEntry(QuestID, UpdatedEntry))
		{
			// HUD 등 PUSH를 구독 중인 UI에게 DTO를 브로드캐스트
			OnQuestEntryUpdated.Broadcast(UpdatedEntry);
		}
	}
}

void UQuestManagerSubSystem::OnPlayerLogin()
{
	// USaveGame이 로드될 때, PlayerQuestHistory를 복원합니다.
}

void UQuestManagerSubSystem::AcceptQuest(FGameplayTag QuestID)
{
	// 퀘스트를 수락하고 진행 상태로 변경합니다.
	PlayerQuestHistory.Emplace(QuestID, EQuestProgress::InProgress);
}

void UQuestManagerSubSystem::ClaimQuestReward(FGameplayTag QuestID)
{
	// 완료된 퀘스트의 보상을 청구합니다.
	EQuestProgress QuestProgress = PlayerQuestHistory.FindRef(QuestID).ProgressType;
	if (QuestProgress == EQuestProgress::Completed_PendingTurnIn)
	{
		FQuestProgressData& ProgressData = PlayerQuestHistory[QuestID];
		ProgressData.ProgressType = EQuestProgress::Complete_Final;
		// 퀘스트 오브젝트 비활성화 및 제거
		// ...
		// DeactivateAndDestroyQuest(QuestObject);
	}
}

void UQuestManagerSubSystem::OnRaidStart()
{
	for (auto& QuestPair : PlayerQuestHistory)
	{
		FQuestProgressData& ProgressData = QuestPair.Value;
		if (ProgressData.ProgressType == EQuestProgress::InProgress)
		{
			LoadAndActivateQuest(QuestPair.Key, &ProgressData);
		}
	}
}

void UQuestManagerSubSystem::OnRaidEnd()
{
	for (UAEQuestObject* QuestObject : ActiveQuests)
	{
		DeactivateAndDestroyQuest(QuestObject);
	}
	ActiveQuests.Empty();
}

void UQuestManagerSubSystem::LoadAndActivateQuest(FGameplayTag QuestID, FQuestProgressData* ProgressData)
{
	// UAssetManager를 사용하여 퀘스트 정의를 로드합니다.
	UAssetManager& AssetManager = UAssetManager::Get();

	FPrimaryAssetId QuestAssetId(UDA_QuestBase::StaticClass()->GetFName(), QuestID.GetTagName());
	UDA_QuestBase* QuestDef = AssetManager.GetPrimaryAssetObject<UDA_QuestBase>(QuestAssetId);

	// 퀘스트 정의가 유효한지 확인합니다.
	if (QuestDef)
	{
		// 새로운 UAEQuestObject를 생성하고 초기화합니다.
		UAEQuestObject* NewQuestObject = NewObject<UAEQuestObject>(this);
		NewQuestObject->Initialize(QuestDef, ProgressData, this);
		// 퀘스트를 활성화합니다.
		NewQuestObject->Activate(this->GetWorld());
		// 활성화된 퀘스트 목록에 추가합니다.
		ActiveQuests.Add(NewQuestObject);
	}
}

void UQuestManagerSubSystem::DeactivateAndDestroyQuest(UAEQuestObject* QuestObject)
{
	// 퀘스트를 비활성화합니다.
	QuestObject->DeActivate();
	ActiveQuests.Remove(QuestObject);

	// 퀘스트 오브젝트를 가비지 컬렉션 대상으로 표시합니다.
	QuestObject->MarkAsGarbage();
}

bool UQuestManagerSubSystem::BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const
{
	const FQuestProgressData* ProgressData = PlayerQuestHistory.Find(QuestID);
	if (ProgressData == nullptr)
	{
		return false; // 진행도 데이터 없음
	}

	return BuildQuestLogEntry(QuestID, *ProgressData, OutEntry);
}

bool UQuestManagerSubSystem::BuildQuestLogEntry(const FGameplayTag& QuestID, const FQuestProgressData& ProgressData, FQuestLogEntry& OutEntry) const
{
	// 에셋 매니저로 UDA_QuestBase 로드
	// DTO 필드 채우기 (Title, Description, State...)
	// FormattedObjectives 텍스트 포맷팅
	// ...
	// 이 모든 로직이 '여기 한 곳'에만 존재합니다.

	return true; // (모든 로직이 성공했다고 가정)
}