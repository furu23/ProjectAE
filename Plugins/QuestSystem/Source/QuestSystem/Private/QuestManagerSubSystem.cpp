// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestManagerSubSystem.h"
#include "QuestSystem.h"
#include "QuestTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Data/DA_QuestBase.h"
#include "QuestObject.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void UQuestManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	PlayerQuestHistory.Empty();

	FQuestProgressData QuestData;
	QuestData.ProgressType = EQuestProgress::CanAccept;
	PlayerQuestHistory.Add(FGameplayTag::RequestGameplayTag("Quest.Id.Interact.GetBox"), QuestData);

	FQuestProgressData QuestData2;
	QuestData.ProgressType = EQuestProgress::Completed_PendingTurnIn;
	PlayerQuestHistory.Add(FGameplayTag::RequestGameplayTag("Quest.Id.Interact.GetBox2"), QuestData2);

	FQuestProgressData QuestData3;
	QuestData.ProgressType = EQuestProgress::NotStarted;
	PlayerQuestHistory.Add(FGameplayTag::RequestGameplayTag("Quest.Id.Interact.GetBox3"), QuestData3);
}

void UQuestManagerSubSystem::OnSystemReady(FGameplayTag NewPhase)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	
	// "QuestData" 타입의 모든 에셋 ID 목록을 가져옵니다.
	TArray<FPrimaryAssetId> AssetIdList;
	AssetManager.GetPrimaryAssetIdList(FName("QuestData"), AssetIdList);

	if (AssetIdList.Num() == 0)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("No QuestData assets found to load."));
		return; // 로드할 것이 없음
	}

	FStreamableDelegate OnLoadCompleteDelegate = FStreamableDelegate::CreateUObject(this, &UQuestManagerSubSystem::OnQuestDataLoaded);
	LoadHandle = AssetManager.LoadPrimaryAssets(AssetIdList, TArray<FName>(), OnLoadCompleteDelegate);

	// 태그가 레이드 진행 중이라면
	if (NewPhase.MatchesTag(FGameplayTag::RequestGameplayTag("Game.Phase.InRaid.GroundZero")))
	{
		OnRaidStart();
	}
	// 레이드가 끝났다면 (탈출/사망)
	else if (NewPhase.MatchesTag(FGameplayTag::RequestGameplayTag("Game.Phase.Lobby")))
	{
		OnRaidEnd();
	}
}

void UQuestManagerSubSystem::OnQuestDataLoaded()
{
	UE_LOG(LogQuestSystem, Log, TEXT("All QuestData assets are now loaded. Caching..."));

	// 로드 요청했던 목록을 다시 가져오거나, 멤버 변수로 저장해둔 목록을 순회합니다.
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIdList;
	AssetManager.GetPrimaryAssetIdList(FName("QuestData"), AssetIdList);

	ActiveQuestDACaches.Empty(AssetIdList.Num());

	for (const FPrimaryAssetId& AssetId : AssetIdList)
	{
		// 로드가 완료되었으므로, GetPrimaryAssetObject는 즉시 유효한 UObject*를 반환합니다.
		UDA_QuestBase* QuestData = Cast<UDA_QuestBase>(AssetManager.GetPrimaryAssetObject(AssetId));

		if (QuestData)
		{
			// FName(ID)을 키로 TMap에 캐싱합니다.
			ActiveQuestDACaches.Add(QuestData->QuestID, QuestData);
		}
	}

	UE_LOG(LogQuestSystem, Log, TEXT("Caching complete. %d quests loaded."), ActiveQuestDACaches.Num());
}

TArray<FQuestLogEntry> UQuestManagerSubSystem::GetQuestLogEntries() const
{
	TArray<FQuestLogEntry> LogEntries;
	for (const TPair<FGameplayTag, FQuestProgressData>& Pair : PlayerQuestHistory)
	{
		FQuestLogEntry Entry;
		if (BuildQuestLogEntry(Pair.Key, Entry))
		{
			LogEntries.Add(Entry);
		}
	}
	return LogEntries;
}

void UQuestManagerSubSystem::NotifyQuestUpdate(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] NotifyQuestUpdate: [%s] Id is Updated..."), *QuestID.GetTagName().ToString());

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

void UQuestManagerSubSystem::AcceptQuest(FGameplayTag QuestID)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] AcceptQuest: [%s] Id is Accepting Now..."), *QuestID.GetTagName().ToString());

	// 퀘스트를 수락하고 진행 상태로 변경합니다.
	PlayerQuestHistory.Emplace(QuestID, EQuestProgress::InProgress);

	OnRaidStart();
}

void UQuestManagerSubSystem::GetSaveData(TArray<uint8>& OutData)
{
	FMemoryWriter MemWriter(OutData, true);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);

	// SaveGame 태그가 붙은 변수만 골라서 직렬화
	Ar.ArIsSaveGame = true;

	// 이 객체의 변수들을 Ar에 씀
	Serialize(Ar);
}

void UQuestManagerSubSystem::LoadSaveData(const TArray<uint8>& InData)
{
	if (InData.Num() == 0) return;

	FMemoryReader MemReader(InData, true);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);

	Ar.ArIsSaveGame = true;

	// Ar에서 읽어서 이 객체 변수에 덮어씌움
	Serialize(Ar);
}	

FQuestProgressData* UQuestManagerSubSystem::QueryProgressDataForQuestId(const FGameplayTag& QuestId)
{
	if (!QuestId.IsValid())
	{
		return nullptr;
	}

	return PlayerQuestHistory.Find(QuestId);
}

void UQuestManagerSubSystem::ClaimQuestReward(FGameplayTag QuestID)
{
	// 완료된 퀘스트의 보상을 청구합니다.
	EQuestProgress QuestProgress = PlayerQuestHistory.FindRef(QuestID).ProgressType;
	if (QuestProgress == EQuestProgress::Completed_PendingTurnIn)
	{
		FQuestProgressData& ProgressData = PlayerQuestHistory[QuestID];
		ProgressData.ProgressType = EQuestProgress::Complete_Final;
		// 보상 수령 로직...

		// if (FRewardData = ensure(ActiveQuestDACaches[QuestID]->RewardData))
		{

		}
	}
}

void UQuestManagerSubSystem::OnRaidStart()
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] In-Raid Level is started now"));

	for (auto& QuestPair : PlayerQuestHistory)
	{
		FQuestProgressData& ProgressData = QuestPair.Value;
		if (ProgressData.ProgressType == EQuestProgress::InProgress)
		{
			LoadAndActivateQuest(QuestPair.Key);
		}
	}
}

void UQuestManagerSubSystem::OnRaidEnd()
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] In-Raid Level is ended now"));

	for (UQuestObject* QuestObject : ActiveQuests)
	{
		DeactivateAndDestroyQuest(QuestObject);
	}
	ActiveQuests.Empty();
}

void UQuestManagerSubSystem::LoadAndActivateQuest(FGameplayTag QuestID)
{
	if (!QuestID.IsValid())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("[QuestSys] LoadAndActivateQuest: QuestID is invalid."));
		return;
	}

	UDA_QuestBase* QuestDef = ActiveQuestDACaches.FindRef(QuestID);
	if (!QuestDef)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] ActivateQuestObject called with NULL QuestDef."));
		return;
	}

	UWorld* World = GetWorld(); // Subsystem은 GetWorld()를 호출할 수 있습니다.
	if (!World)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cannot activate quest [%s], World is NULL."), *QuestDef->GetPrimaryAssetId().PrimaryAssetName.ToString());
		return;
	}

	// 새로운 UAEQuestObject를 생성하고 초기화합니다.
	UQuestObject* NewQuestObject = NewObject<UQuestObject>(this);
	NewQuestObject->Initialize(QuestDef, this);

	// 델리게이트를 바인딩합니다.
	NewQuestObject->OnQuestObjectChangedDelegate.BindUObject(this, &UQuestManagerSubSystem::NotifyQuestUpdate);
	// NewQuestObject->OnRequestWorldTasksDelegate.BindUObject(this, &UQuestManagerSubSystem::OnQuestRequestingWorldTasks);

	// 퀘스트를 활성화합니다.
	NewQuestObject->Activate(World);

	// 활성화된 퀘스트 목록에 추가합니다.
	ActiveQuests.Add(NewQuestObject);

	// --- 기존 로직 끝 ---

	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] ActivateQuestObject: [%s] Id is activated completely"), *QuestDef->GetPrimaryAssetId().PrimaryAssetName.ToString());
}

void UQuestManagerSubSystem::DeactivateAndDestroyQuest(UQuestObject* QuestObject)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] DeactivateAndDestroyQuest: Object [%s] is deactivated"), *QuestObject->GetFName().ToString());

	// 퀘스트를 비활성화합니다.
	QuestObject->DeActivate();
	ActiveQuests.Remove(QuestObject);

	// 퀘스트 오브젝트를 가비지 컬렉션 대상으로 표시합니다.
	QuestObject->MarkAsGarbage();
}

/*
void UQuestManagerSubSystem::OnQuestRequestingWorldTasks(const TArray<TObjectPtr<UQuestWorldTask>> TasksToExecute)
{
	if (UQuestWorldDirector* WorldDirector = GetWorld()->GetSubsystem<UQuestWorldDirector>())
		(
			WorldDirector->ExecuteTasks(TasksToExecute);
			)
	else
		(
			UE_LOG(LogTemp, Warning, TEXT("UQuestWorldDirector가 없어 태스크를 실행할 수 없습니다!"));
			)
}*/

bool UQuestManagerSubSystem::BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const
{
	const FQuestProgressData ProgressData = PlayerQuestHistory.FindRef(QuestID);

	if (!QuestID.IsValid()) 
	{
		return false;
	}
	UDA_QuestBase* QuestDef = *ActiveQuestDACaches.Find(QuestID);
	if (!QuestDef)
	{
		OutEntry.QuestID = QuestDef->QuestID;
		OutEntry.Title = QuestDef->QuestName;
		OutEntry.Description = QuestDef->Description;
		OutEntry.CurrentState = ProgressData.ProgressType;

		OutEntry.FormattedObjectives.Empty(QuestDef->ObjectConfigs.Num());

		for (const UQuestObjectiveConfig* ObjConfig : QuestDef->ObjectConfigs)
		{
			if (ObjConfig)
			{
				FText FormattedText = ObjConfig->GetFormattedObjectiveText(ProgressData);
				OutEntry.FormattedObjectives.Add(FormattedText);
			}
		}
	}
	else
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] BuildQuestLogEntry: QuestID [%s] DA is not found!"), *QuestID.GetTagName().ToString());
		return false; // 실패
	}
	return true;
}