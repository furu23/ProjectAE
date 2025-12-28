#include "QuestComponent.h"
#include "QuestSystem.h"
#include "QuestTypes.h"
#include "Data/QuestObjectConfig.h"
#include "Object/QuestObject.h"
#include "Action/QuestAction.h"

#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

// ================================================================
// FQuestFastArray
// 
// 퀘스트 진행도 데이터를 빠르게 동기화하기 위한 패스트 배열 시리얼라이저 구현체입니다.
// ================================================================


// ----------------------------------------------------------------
// Interface: FastArraySerializer
// ----------------------------------------------------------------
#pragma region Interface: FastArraySerializer

void FQuestFastArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	bCacheDirtyFlag = true;
}

void FQuestFastArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	bCacheDirtyFlag = true;
}

bool FQuestFastArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FQuestProgressData, FQuestFastArray>(QuestProgressItems, DeltaParms, *this);
}
#pragma endregion


// ----------------------------------------------------------------
// Server-Side Operations
// ----------------------------------------------------------------
#pragma region Server-Side Operations

bool FQuestFastArray::UpdateProgressData(FQuestProgressData& InData)
{
	const FGameplayTag& InputKey = InData.GetQuestID();

	if (!InputKey.IsValid()) return false;

	if (bCacheDirtyFlag)
	{
		RebuildCache();
	}

	// 인덱스가 필요하므로
	if (const int32* IdxPtr = QuestIndexMap.Find(InputKey))
	{
		int32 Idx = *IdxPtr;
		if (QuestProgressItems.IsValidIndex(Idx))
		{
			if (QuestProgressItems[Idx].GetQuestID() != InputKey)
			{
				UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Quest ID [%s]"), *InputKey.ToString());
				return false;
			}

			FQuestProgressData& TargetItem = QuestProgressItems[Idx];

			if (InData.GetProgressDataVersion() != QUEST_DATA_CURRENT_VERSION)
			{
				InData.MigrateToLatest();
			}

			if (TargetItem.GetProgressDataVersion() != QUEST_DATA_CURRENT_VERSION)
			{
				TargetItem.MigrateToLatest();
			}

			TargetItem.UpdateProgress(InData.GetProgressType());
			TargetItem.UpdateAllObjectives(InData.GetObjectives());

			MarkItemDirty(QuestProgressItems[Idx]);

			return true;
		}
	}
	else
	{
		AddItem(InData);
		return true;
	}
}

bool FQuestFastArray::UpdateProgressData(const FGameplayTag& QuestID, EQuestProgress ProgressType)
{
	if (!QuestID.IsValid()) return false;

	if (ProgressType == EQuestProgress::None) return false;

	FQuestProgressData* FindProgressData = Internal_Find(QuestID);
	if (!FindProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Quest ID [%s]"), *QuestID.ToString());
		return false;
	}

	FindProgressData->UpdateProgress(ProgressType);
	MarkItemDirty(*FindProgressData);

	return true;
}

bool FQuestFastArray::UpdateProgressData(const FGameplayTag& QuestID, const FGameplayTag& ObjID, int32 NewValue)
{
	if (!QuestID.IsValid() || !ObjID.IsValid()) return false;

	FQuestProgressData* FindProgressData = Internal_Find(QuestID);
	if (!FindProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Quest ID [%s]"), *QuestID.ToString());
		return false;
	}

	if (!FindProgressData->UpdateObjective(ObjID, NewValue))
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Objective ID [%s]"), *ObjID.ToString());
		return false;
	}

	MarkItemDirty(*FindProgressData);

	return true;
}

bool FQuestFastArray::UpdateProgressData(const FGameplayTag& QuestID, const FGameplayTag& ObjID, EQuestProgress ProgressType, int32 NewValue)
{
	if (!QuestID.IsValid() || !ObjID.IsValid()) return false;

	FQuestProgressData* FindProgressData = Internal_Find(QuestID);
	if (!FindProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Quest ID [%s]"), *QuestID.ToString());
		return false;
	}

	if (!FindProgressData->UpdateObjective(ObjID, NewValue))
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Objective ID [%s]"), *ObjID.ToString());
		return false;
	}

	FindProgressData->UpdateProgress(ProgressType);
	MarkItemDirty(*FindProgressData);

	return true;
}

void FQuestFastArray::InitializeFromSaveData(const TSet<FGameplayTag>& ValidQuestTags)
{
	// 데이터 변경 체크 플래그
	bool bStructureChanged = false;

	// 기존 데이터 정리 및 마이그레이션
	for (int32 i = QuestProgressItems.Num() - 1; i >= 0; --i)
	{
		if (!ValidQuestTags.Contains(QuestProgressItems[i].GetQuestID()))
		{
			QuestProgressItems.RemoveAt(i);
			bStructureChanged = true;
			continue;
		}

		if (QuestProgressItems[i].MigrateToLatest())
		{
			MarkItemDirty(QuestProgressItems[i]);
		}
	}

	// 구조가 변경되었으면 캐시 재구축
	if (bStructureChanged)
	{
		RebuildCache();
	}

	// 누락된 퀘스트 데이터 추가
	bool bAddedNew = false;
	for (const FGameplayTag& ValidTag : ValidQuestTags)
	{

		if (!QuestIndexMap.Contains(ValidTag))
		{
			QuestProgressItems.Emplace(ValidTag, EQuestProgress::NotStarted);
			bAddedNew = true;
		}
	}

	// 새로운 항목이 추가되었으면 캐시 재구축
	if (bAddedNew)
	{
		RebuildCache();
		bStructureChanged = true;
	}

	// 배열 구조가 변경되었으면 MarkArrayDirty 호출
	if (bStructureChanged)
	{
		MarkArrayDirty();
	}
}

void FQuestFastArray::AddItem(const FQuestProgressData& NewItem)
{
	FQuestProgressData& Item = QuestProgressItems.Add_GetRef(NewItem);
	MarkItemDirty(Item);

	bCacheDirtyFlag = true;
}

bool FQuestFastArray::RemoveItem(const FGameplayTag& InQuestID)
{
	if (bCacheDirtyFlag)
	{
		RebuildCache();
	}

	// 캐시를 통해 빠르게 찾음
	if (const int32* IdxPtr = QuestIndexMap.Find(InQuestID))
	{
		int32 Idx = *IdxPtr;
		if (QuestProgressItems.IsValidIndex(Idx) && QuestProgressItems[Idx].GetQuestID() == InQuestID)
		{
			MarkItemDirty(QuestProgressItems[Idx]);
			QuestProgressItems.RemoveAt(Idx);
			MarkArrayDirty();

			bCacheDirtyFlag = true;

			return true;
		}
	}
	return false;
}
#pragma endregion


// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------
#pragma region Public API
void FQuestFastArray::Empty()
{
	QuestProgressItems.Empty();
	MarkArrayDirty();

	// 캐시 리빌드는 InitializeFromSaveData에서 진행하므로
	bCacheDirtyFlag = false;
}
#pragma endregion


// ----------------------------------------------------------------
// Internal Implementation
// ----------------------------------------------------------------
#pragma region Internal Implementation
const FQuestProgressData* FQuestFastArray::Internal_Find(const FGameplayTag& InQuestID) const
{
	if (!InQuestID.IsValid()) return nullptr;

	if (QuestProgressItems.Num() <= LinearSearchThreshold)
	{
		return QuestProgressItems.FindByKey(InQuestID);
	}


	if (bCacheDirtyFlag)
	{
		RebuildCache();
	}

	if (const int32* IdxPtr = QuestIndexMap.Find(InQuestID))
	{
		if (QuestProgressItems.IsValidIndex(*IdxPtr) && QuestProgressItems[*IdxPtr].GetQuestID() == InQuestID)
		{
			return &QuestProgressItems[*IdxPtr];
		}
	}

	return nullptr;
}

FQuestProgressData* FQuestFastArray::Internal_Find(const FGameplayTag& InQuestID)
{
	if (!InQuestID.IsValid()) return nullptr;

	const FQuestProgressData* Result = const_cast<const FQuestFastArray*>(this)->Internal_Find(InQuestID);
	return const_cast<FQuestProgressData*>(Result);
}

void FQuestFastArray::RebuildCache() const
{
	QuestIndexMap.Empty(QuestProgressItems.Num());
	for (int32 i = 0; i < QuestProgressItems.Num(); ++i)
	{
		QuestIndexMap.Add(QuestProgressItems[i].GetQuestID(), i);
	}

	bCacheDirtyFlag = false;
}
#pragma endregion



// ================================================================
// UQuestComponent
// 
// 퀘스트 시스템의 핵심 컴포넌트 클래스입니다.
// ================================================================
UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ----------------------------------------------------------------
// Core Framework
// ----------------------------------------------------------------
#pragma region Core Framework
void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bIsDataLoadedFromSave)
	{
		// 빈 깡통(New Game) 상태로 데이터 준비
		PrepareQuestData();
		bIsDataLoadedFromSave = true; // (사실상 New Game 데이터 로드됨)
	}

	RestoreActiveQuest();


#if !UE_BUILD_SHIPPING
	IConsoleManager& ConsoleMgr = IConsoleManager::Get();

	ConsoleCommands.Add(ConsoleMgr.RegisterConsoleCommand(
		TEXT("Quest.ForceComplete"),
		TEXT("Force completes a quest. Usage: Quest.ForceComplete <QuestID>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UQuestComponent::Console_ForceCompleteQuest),
		ECVF_Cheat
	));

	ConsoleCommands.Add(ConsoleMgr.RegisterConsoleCommand(
		TEXT("Quest.ForceCompleteObj"),
		TEXT("Force completes a specific objective. Usage: Quest.ForceCompleteObj <QuestID> <ObjID>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UQuestComponent::Console_ForceCompleteQuestObj),
		ECVF_Cheat
	));

	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] Debug Console Commands Registered."));
#endif
}

void UQuestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto& Pair : LoadHandles)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->ReleaseHandle();
		}
	}

#if !UE_BUILD_SHIPPING
	for (IConsoleObject* Cmd : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Cmd);
	}
	ConsoleCommands.Empty();
#endif
}

void UQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UQuestComponent, QuestProgressList, COND_OwnerOnly);
}
#pragma endregion



// ----------------------------------------------------------------
// Public Queries
// ----------------------------------------------------------------
#pragma region Public Queries
/*
TArray<FQuestLogEntry> UQuestComponent::GetQuestLogEntries() const
{
	TArray<FQuestLogEntry> LogEntries;

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] GetQuestLogEntries: [%d] in QuestProgressList..."), QuestProgressList.Num());
	for (const FQuestProgressData ProgressData : QuestProgressList)
	{
		FQuestLogEntry Entry;
		if (BuildQuestLogEntry(ProgressData.QuestID, Entry))
		{
			LogEntries.Add(Entry);
		}
	}
	return LogEntries;
}
*/


TArray<FQuestLogEntry> UQuestComponent::GetQuestListSummary() const
{

}

void UQuestComponent::RequestQuestDetail(FGameplayTag QuestID)
{

}

bool UQuestComponent::HasActiveQuest(const FGameplayTag& QuestID) const
{
	return ActiveQuests.ContainsByPredicate([&](const UQuestObject* Quest)
		{
			return Quest && Quest->GetQuestID() == QuestID;
		});
}

const UQuestObject* UQuestComponent::FindActiveQuest(const FGameplayTag& QuestID) const
{
	auto* ResultPtr = ActiveQuests.FindByPredicate([&](const auto& Quest)
		{
			return Quest && Quest->GetQuestID() == QuestID;
		});

	return ResultPtr ? ResultPtr->Get() : nullptr;
}

const FQuestProgressData* UQuestComponent::QueryProgressDataForQuestID(const FGameplayTag& QuestID) const
{
	if (!QuestID.IsValid())
	{
		return nullptr;
	}

	return QuestProgressList.Find(QuestID);
}
#pragma endregion


// ----------------------------------------------------------------
// Quest Flow Control
// ----------------------------------------------------------------
#pragma region Quest Flow Actions: Accept Quest
void UQuestComponent::RequestAcceptQuest(const FGameplayTag& QuestID)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_AcceptQuest(QuestID);
		LoadQuestObjectData(QuestID, FStreamableDelegate::CreateWeakLambda(this, [this, QuestID](){}));
	}
	else
	{
		Internal_AcceptQuest(QuestID);
	}
}

void UQuestComponent::Server_AcceptQuest_Implementation(const FGameplayTag& QuestID)
{
	Internal_AcceptQuest(QuestID);
}

bool UQuestComponent::Server_AcceptQuest_Validate(const FGameplayTag& QuestID)
{
	if (!ValidateAcceptQuest(QuestID))
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Server_AcceptQuest_Validate: Validation failed for QuestID [%s]"), *QuestID.GetTagName().ToString());
		return false;
	}

	return true;
}

void UQuestComponent::Internal_AcceptQuest(const FGameplayTag& QuestID)
{
	if (!ValidateAcceptQuest(QuestID))
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("AcceptQuest Failed: Invalid ID or Metadata missing for [%s]"), *QuestID.ToString());
		Client_AcceptQuest_Rejected(QuestID);
		return;
	}

	// 이미 활성화된 퀘스트인지 검사
	if (HasActiveQuest(QuestID))
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] RequestAcceptQuest: QuestID [%s] is already active"), *QuestID.GetTagName().ToString());
		Client_AcceptQuest_Rejected(QuestID);
		return;
	}

	// 진행 상태 검사
	const FQuestProgressData* ExistingData = QuestProgressList.Find(QuestID);
	if (ExistingData)
	{
		if (ExistingData->GetProgressType() != EQuestProgress::CanAccept)
		{
			UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] AcceptQuest: [%s] ID has not validate data"), *QuestID.GetTagName().ToString());
			return;
		}
	}
	else
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] AcceptQuest: [%s] ID has not validate data"), *QuestID.GetTagName().ToString());
		return;
	}

	// 실제 수락 처리
	AcceptQuest(QuestID);


	// BP 이벤트 방생
	K2_OnQuestAccepted(QuestID);
	// RPC, 클라이언트 알림
	Client_AcceptQuest(QuestID);
}

bool UQuestComponent::ValidateAcceptQuest(const FGameplayTag& QuestID) const
{
	if (!QuestID.IsValid()) return false;
	if (!QuestMetadataCache.Contains(QuestID)) return false;

	return true;
}

void UQuestComponent::AcceptQuest(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] AcceptQuest: [%s] Id is Accepting Now..."), *QuestID.GetTagName().ToString());

	// 퀘스트를 수락하고 진행 상태로 변경합니다.
	QuestProgressList.UpdateProgressData(QuestID, EQuestProgress::InProgress);
	
	LoadQuestObjectData(QuestID, FStreamableDelegate::CreateWeakLambda(this, [this, QuestID]()
		{
			UAssetManager& AssetMgr = UAssetManager::Get();
			FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());

			if (UQuestObjectConfig* QuestData = Cast<UQuestObjectConfig>(AssetMgr.GetPrimaryAssetObject(AssetID)))
			{
				UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Quest [%s] Accepted! Actions Executed."), *QuestID.ToString());

				TArray<TObjectPtr<UQuestAction>> OutActions;
				FQuestContext Context = UQuestHelpers::MakeQuestContext(this, this->GetOwner(), nullptr, FGameplayTagContainer::EmptyContainer, FGameplayTag::EmptyTag, 1);
				ExecuteQuestActions(QuestData->QuestAcceptAction, QuestID, Context, ENetworkActionType::ServerOnly);
			}
			StartActivateQuest(QuestID);
		}
	));
}

void UQuestComponent::Client_AcceptQuest_Implementation(const FGameplayTag& QuestID)
{
	LoadQuestObjectData(QuestID, FStreamableDelegate::CreateWeakLambda(this, [this, QuestID]()
		{
			UAssetManager& AssetMgr = UAssetManager::Get();
			FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());

			if (UQuestObjectConfig* QuestData = Cast<UQuestObjectConfig>(AssetMgr.GetPrimaryAssetObject(AssetID)))
			{
				UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Quest [%s] Accepted! Actions Executed."), *QuestID.ToString());

				TArray<TObjectPtr<UQuestAction>> OutActions;
				FQuestExecutionContext Context();
				ExecuteQuestActions(QuestData->QuestAcceptAction, QuestID, Context, ENetworkActionType::ClientOnly);
			}
		}
	));
}

void UQuestComponent::Client_AcceptQuest_Rejected_Implementation(const FGameplayTag& QuestID)
{
	if (LoadHandles.Contains(QuestID))
	{
		LoadHandles.Remove(QuestID);
		UE_LOG(LogQuestSystem, Warning, TEXT("[QuestSys] Quest [%s] Rejected by Server. Cleanup handles."), *QuestID.ToString());
	}
}
#pragma endregion

#pragma region Quest Flow Actions: Claim Reward
void UQuestComponent::RequestClaimReward(const FGameplayTag& QuestID)
{

}

void UQuestComponent::Server_ClaimReward_Implementation(const FGameplayTag& QuestID)
{

}

bool UQuestComponent::Server_ClaimReward_Validate(const FGameplayTag& QuestID)
{
	return true;
}

void UQuestComponent::Internal_ClaimQuestReward(const FGameplayTag& QuestID)
{

}

bool UQuestComponent::CanClaimQuestReward(const FGameplayTag& QuestID) const
{

}

bool UQuestComponent::CanClaimQuestReward_Native(const FGameplayTag& QuestID) const
{
	return true;
}

void UQuestComponent::OnPreClaimQuestReward(const FGameplayTag& QuestID)
{

}

void UQuestComponent::OnPostClaimQuestReward(const FGameplayTag& QuestID)
{

}

void UQuestComponent::ClaimQuestReward(const FGameplayTag& QuestID)
{
	// 퀘스트가 보상 대기 상태인지 검사
	const FQuestProgressData* CurrentQuestData = QuestProgressList.Find(QuestID);
	if (!CurrentQuestData || CurrentQuestData->GetProgressType() != EQuestProgress::Completed_PendingTurnIn)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] ClaimQuestReward: [%s] ID has not validate data"), *QuestID.GetTagName().ToString());
		return;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] ClaimQuestReward: Complete Quest for [%s] ID"), *QuestID.GetTagName().ToString());
	QuestProgressList.UpdateProgressData(QuestID, EQuestProgress::Complete_Final);

	FQuestLogEntry UpdatedEntry;
	if (BuildQuestLogEntry(QuestID, UpdatedEntry))
	{
		OnQuestEntryUpdatedDelegate.Broadcast(UpdatedEntry);
	}

	// 퀘스트 보상 제공
	GiveReward(QuestID);

	// 후행 퀘스트 상태 전이 확인
	TryUnlockNextQuests(QuestID);
	K2_OnQuestRewardClaimed(QuestID);
}

void UQuestComponent::Client_ClaimQuestReward_Implementation(const FGameplayTag& QuestID)
{

}
#pragma endregion

#pragma region Quest Flow Actions: Abandon Quest
void UQuestComponent::RequestAbandonQuest(const FGameplayTag& QuestID)
{

}

void UQuestComponent::Server_AbandonQuest_Implementation(const FGameplayTag& QuestID)
{

}

bool UQuestComponent::Server_AbandonQuest_Validate(const FGameplayTag& QuestID)
{
	return true;
}

void UQuestComponent::Internal_AbandonQuest(const FGameplayTag& QuestID)
{

}

bool UQuestComponent::CanAbandonQuest(const FGameplayTag& QuestID) const
{

}

bool UQuestComponent::CanAbandonQuest_Native(const FGameplayTag& QuestID) const
{

}

void UQuestComponent::OnPreAbandonQuest(const FGameplayTag& QuestID)
{

}

void UQuestComponent::OnPostAbandonQuest(const FGameplayTag& QuestID)
{

}

void UQuestComponent::AbandonQuest(const FGameplayTag& QuestID)
{

}

void UQuestComponent::Client_AbandonQuest_Implementation(const FGameplayTag& QuestID)
{

}
#pragma endregion

#pragma region Quest Flow Actions: Helpers
void UQuestComponent::GiveReward(const FGameplayTag& QuestID)
{
	const UQuestObject* CurrentQuest = FindActiveQuest(QuestID);

	if (!ensureMsgf(CurrentQuest, TEXT("GiveReward: Active Quest missing for [%s]"), *QuestID.ToString()))
	{
		return;
	}

	const UQuestObjectConfig* CurrentConfig = CurrentQuest->GetQuestDefinition();
	for (UQuestAction* Action : CurrentConfig->QuestClaimRewardAction)
	{
		if (Action) Action->ExecuteAction(this, QuestID);
	}
}

void UQuestComponent::TryUnlockNextQuests(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] TryUnlockNextQuests: Unlock Quest Next of [%s] ID"), *QuestID.GetTagName().ToString());

	const UQuestObject* CurrentQuest = FindActiveQuest(QuestID);
	if (!CurrentQuest)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] GiveReward: Cannot find Active QuestObject for [%s]"), *QuestID.ToString());
		return;
	}

	const UQuestObjectConfig* CurrentConfig = CurrentQuest->GetQuestDefinition()
		if (!ensureAlwaysMsgf(CurrentQuestDEf, TEXT("Quest Data Missing for ID: %s"), *QuestID.ToString()))
		{
			return;
		}

	const FGameplayTagContainer& NextQuests = CurrentQuestDEf->PostrequisiteQuests;

	if (NextQuests.IsValid())
	{
		for (const FGameplayTag& NextID : NextQuests)
		{
			if (!NextID.IsValid()) continue;

			FQuestProgressData* ExistingData = QuestProgressList.Find(NextID);
			if (ExistingData)
			{
				if (ExistingData->ProgressType != EQuestProgress::None &&
					ExistingData->ProgressType != EQuestProgress::NotStarted)
				{
					continue;
				}
			}

			UQuestObjectConfig* NextQuestObjPtr = ActiveQuestDACaches.FindRef(NextID);
			if (!ensure(NextQuestObjPtr)) continue;

			const FGameplayTagContainer& Prerequisites = NextQuestObjPtr->PrerequisiteQuests;

			bool bAllPrereqsMet = true;
			for (const FGameplayTag& PrereqID : Prerequisites)
			{
				const FQuestProgressData* PrereqHistory = QuestProgressList.Find(PrereqID);

				// 기록이 아예 없거나, 최종 완료 상태가 아니라면 탈락
				if (!PrereqHistory || PrereqHistory->ProgressType != EQuestProgress::Complete_Final)
				{
					bAllPrereqsMet = false;
					break;
				}
			}

			if (bAllPrereqsMet)
			{
				FQuestProgressData* NewQuestData = QuestProgressList.Find(NextID);
				if (NewQuestData)
				{
					if (NewQuestData->ProgressType == EQuestProgress::NotStarted)
					{
						UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] TryUnlockNextQuests: Unlock New Quest [%s] Successfully"), *NextID.GetTagName().ToString());

						NewQuestData->ProgressType = EQuestProgress::CanAccept;


						FQuestLogEntry UpdatedEntry;
						if (BuildQuestLogEntry(NextID, UpdatedEntry))
						{
							OnQuestEntryUpdatedDelegate.Broadcast(UpdatedEntry);
						}
					}
				}
			}
		}
	}
}
#pragma endregion


// ----------------------------------------------------------------
// Save & Load
// ----------------------------------------------------------------
#pragma region Save & Load
void UQuestComponent::GetSaveData(TArray<uint8>& OutData) const
{
	FMemoryWriter MemWriter(OutData, true);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);

	// SaveGame 태그가 붙은 변수만 골라서 직렬화
	Ar.ArIsSaveGame = true;

	// 이 객체의 변수들을 Ar에 씀
	Serialize(Ar);
}

void UQuestComponent::LoadSaveData(const TArray<uint8>& InData)
{
	if (InData.Num() == 0) return;

	FMemoryReader MemReader(InData, true);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);

	Ar.ArIsSaveGame = true;

	// Ar에서 읽어서 이 객체 변수에 덮어씌움
	Serialize(Ar);

	// 로드 후 초기화 작업
	PrepareQuestData();
	bIsDataLoadedFromSave = true;

	// 활성 퀘스트 시작
	if (HasBegunPlay())
	{
		StopActiveQuests();
		RestoreActiveQuest();
	}
}

FQuestPreloadHandle UQuestComponent::RequestPreloadQuestData(const FGameplayTag& QuestID)
{
	FQuestPreloadHandle OutHandle;

	if (!QuestID.IsValid()) return OutHandle;

	UAssetManager& AssetManager = UAssetManager::Get();
	FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());

	TSharedPtr<FStreamableHandle> NativeHandle = AssetManager.LoadPrimaryAsset(AssetID, TArray<FName>(), FStreamableDelegate());

	if (NativeHandle.IsValid())
	{
		OutHandle.NativeHandle = NativeHandle;

		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Preload: Handle Struct created for [%s]."), *QuestID.ToString());
	}

	return OutHandle;
}

void UQuestComponent::PrepareQuestData()
{
	// 메타데이터가 로드되지 않은 경우 로드
	if (!bHasLoadedQuestMetadata)
	{
		LoadQuestMetadataTable();
		bHasLoadedQuestMetadata = true;
	}

	// 유효 태그 집합 생성
	TSet<FGameplayTag> ValidQuestTags;
	for (TPair<FGameplayTag, FQuestTableRow>& QuestMetadata : QuestMetadataCache)
	{
		if (!QuestMetadata.Key.IsValid()) continue;

		ValidQuestTags.Add(QuestMetadata.Key);
	}

	// 진행도 초기화
	QuestProgressList.InitializeFromSaveData(ValidQuestTags);
}

void UQuestComponent::LoadQuestMetadataTable()
{
	if (!QuestMetadataTable)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] LoadQuestMetadataTable: QuestMetadataTable is not assigned in QuestComponent."));
		return;
	}

	QuestMetadataCache.Empty();

	for (const auto& RowPair : QuestMetadataTable->GetRowMap())
	{
		const FName& RowName = RowPair.Key;
		FQuestTableRow* RowData = reinterpret_cast<FQuestTableRow*>(RowPair.Value);
		if (RowData)
		{
			FGameplayTag QuestTag = FGameplayTag::RequestGameplayTag(RowName, false);
			if (!QuestTag.IsValid())
			{
				UE_LOG(LogQuestSystem, Warning, TEXT("[QuestSys] LoadQuestMetadataTable: Invalid QuestTag for Row [%s]. Skipping."), *RowName.ToString());
				continue;
			}

			QuestMetadataCache.Add(QuestTag, *RowData);
		}
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] LoadQuestMetadataTable: Loaded [%d] Quest Metadata entries."), QuestMetadataCache.Num());
}

void UQuestComponent::LoadQuestObjectData(const FGameplayTag& QuestID, FStreamableDelegate OnLoadComplete)
{
	PendingLoadCallbacks.FindOrAdd(QuestID).Add(OnLoadComplete);

	// 액티브 상태인 핸들을 확인
	if (TSharedPtr<FStreamableHandle>* HandlePtr = LoadHandles.Find(QuestID))
	{
		TSharedPtr<FStreamableHandle> ExistingHandle = *HandlePtr;
		if (ExistingHandle.IsValid())
		{
			// 로드가 끝났다면 실행, 이외엔 대기
			if (ExistingHandle->HasLoadCompleted())
			{
				ProcessPendingCallbacks(QuestID);
			}
			return;
		}
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());

	// 유효성 검사
	if (!AssetManager.GetPrimaryAssetPath(AssetID).IsValid())
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] LoadAsset: Invalid AssetID for [%s]"), *QuestID.ToString());

		// 데이터 유효성이 보장되지 않은 상태에서 실행하면 치명적 에러, 퀘스트가 동작하지 않도록 삭제하는 정책
		PendingLoadCallbacks.Remove(QuestID);
		return;
	}

	auto WrapperDelegate = [this, QuestID]()
		{
			// 로드 완료 시 핸들 정리
			LoadHandles.Remove(QuestID);

			// 콜백 실행
			ProcessPendingCallbacks(QuestID);
			UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] LoadAsset: Async load finished for [%s]."), *QuestID.ToString());
		};

	// 에셋 로드 요청
	TSharedPtr<FStreamableHandle> NewHandle = AssetManager.LoadPrimaryAsset(AssetID, TArray<FName>(), FStreamableDelegate::CreateWeakLambda(this, WrapperDelegate));
	if (NewHandle.IsValid() && !NewHandle->HasLoadCompleted())	// 이미 로드되어 있을 경우를 제외
	{
		LoadHandles.Add(QuestID, NewHandle);
	}
}

void UQuestComponent::ProcessPendingCallbacks(const FGameplayTag& QuestID)
{
	if (TArray<FStreamableDelegate>* FoundCallbacks = PendingLoadCallbacks.Find(QuestID))
	{
		TArray<FStreamableDelegate> CallbacksForFire = *FoundCallbacks;
		PendingLoadCallbacks.Remove(QuestID);

		for (const FStreamableDelegate& Callback : CallbacksForFire)
		{
			Callback.ExecuteIfBound();
		}
	}
}

void UQuestComponent::StartActivateQuest(const FGameplayTag& QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for StartActivateQuest. Need Authority."));
		return;
	}

	if (!ensure(QuestID.IsValid())) return;

	if (HasActiveQuest(QuestID))
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cannot activate quest [%s], Quest is already Activated."), *QuestID.ToString());
		return;
	}

	// 로드 완료된 객체 가져옴
	UAssetManager& AssetManager = UAssetManager::Get();
	FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());

	UQuestObjectConfig* QuestData = Cast<UQuestObjectConfig>(AssetManager.GetPrimaryAssetObject(AssetID));

	// 새로운 UQuestObject를 생성하고 초기화
	if (!ensureMsgf(QuestData && QuestData->QuestObjectClass, TEXT("Invalid DataAsset for %s"), *QuestID.ToString()))
	{
		return;
	}
	UQuestObject* NewQuestObject = NewObject<UQuestObject>(this, QuestData->QuestObjectClass);

	if (!NewQuestObject->Initialize(QuestData, this))
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] QuestObject is Failed to Initialized!"), *QuestID.ToString());
		return;
	}

	NewQuestObject->OnQuestProgressChangedDelegate.BindUObject(this, &UQuestComponent::OnQuestProgressUpdated);
	NewQuestObject->OnQuestCompletionMetDelegate.BindUObject(this, &UQuestComponent::OnQuestObjectCompelete);

	NewQuestObject->Activate();

	ActiveQuests.Add(NewQuestObject);
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] StartActivateQuest: Quest [%s] is activated completely"), *QuestID.ToString());
}

void UQuestComponent::DeactivateAndDestroyQuest(const FGameplayTag& QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for StartActivateQuest. Need Authority."));
		return;
	}

	const int32 Index = ActiveQuests.IndexOfByPredicate([&](const UQuestObject* Quest)
		{
			return Quest && Quest->GetQuestID() == QuestID;
		});

	if (Index != INDEX_NONE)
	{
		UQuestObject* QuestObj = ActiveQuests[Index];

		if (QuestObj)
		{
			QuestObj->OnQuestObjectChangedDelegate.Unbind();
			QuestObj->OnRequestWorldTasksDelegate.Unbind();

			UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Deactivate: Object [%s] removed."), *QuestObj->GetFName().ToString());
		}

		ActiveQuests.RemoveAtSwap(Index);
	}
	else
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("[QuestSys] Failed to find quest [%s] to deactivate."), *QuestID.ToString());
	}
}

void UQuestComponent::ExecuteQuestActions(TArray<TObjectPtr<UQuestAction>>& Actions, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext, ENetworkActionType NetPolicy)
{
	for (const TObjectPtr<UQuestAction>& TemplateAction : Actions)
	{
		if (!TemplateAction) continue;

		// 네트워크 정책에 따른 실행 여부 판단
		if ((NetPolicy & TemplateAction->GetNetworkActionType()) != ENetworkActionType::None) return;

		
		switch (TemplateAction->GetInstancingPolicy())
		{
		case EActionInstancingPolicy::NonInstanced:		// stateless
			TemplateAction->ExecuteAction(this, QuestID, QuestContext);
			break;

		case EActionInstancingPolicy::InstancedPerExecution:	// always instancing
			UQuestAction* NewInstance = DuplicateObject<UQuestAction>(TemplateAction, this);
			if (NewInstance)
			{
				ActiveQuestActions.Add(NewInstance);
				NewInstance->ExecuteAction(this, QuestID, QuestContext);
				NewInstance->OnQuestActionEndedDelegate.AddUObject(this, &UQuestComponent::OnQuestActionEnded);
			}
			break;

		case  EActionInstancingPolicy::InstancedPerObject:	// Instancing Only Once
			UQuestAction* FoundAction = nullptr;
			for (UQuestAction* ActiveInst : ActiveQuestActions)
			{
				if (ActiveInst && (ActiveInst->GetArchetype() == TemplateAction))
				{
					FoundAction = ActiveInst;
					break;
				}
			}
			if (FoundAction)
			{
				FoundAction->ExecuteAction(this, QuestID, QuestContext);
			}
			else
			{
				UQuestAction* NewInstance = DuplicateObject<UQuestAction>(TemplateAction, this);
				if (NewInstance)
				{
					NewInstance->ExecuteAction(this, QuestID, QuestContext);
					ActiveQuestActions.Add(NewInstance);
				}
			}

		default:
			break;
		}
	}
}

void UQuestComponent::OnQuestActionEnded(UQuestAction* EndedAction)
{
	ActiveQuestActions.RemoveSingle(EndedAction);
}

void UQuestComponent::OnQuestObjectCompelete(UQuestObject* CompleteQuest)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for OnQuestObjectCompelete. Need Authority."));
		return;
	}

	if (!ensureMsgf(CompleteQuest && CompleteQuest->GetQuestID().IsValid(), TEXT("QuestObject Internal Error")))
	{
		return;
	}

	const FGameplayTag QuestID = CompleteQuest->GetQuestID();
	LoadQuestObjectData(QuestID, FStreamableDelegate::CreateWeakLambda(this, [this, QuestID]()
		{
			if (!ensureMsgf(QuestProgressList.Find(QuestID), TEXT("Critical: Active quest [%s] lost progress data!"), *QuestID.ToString()))
			{
				return;
			}

			QuestProgressList.UpdateProgressData(QuestID, EQuestProgress::Completed_PendingTurnIn);

			UAssetManager& AssetMgr = UAssetManager::Get();
			FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());

			if (UQuestObjectConfig* QuestData = Cast<UQuestObjectConfig>(AssetMgr.GetPrimaryAssetObject(AssetID)))
			{
				FQuestExecutionContext Context = UQuestHelpers::MakeQuestContext(this, this->GetOwner(), nullptr, FGameplayTagContainer::EmptyContainer, FGameplayTag::EmptyTag, 1);
				ExecuteQuestActions(QuestData->QuestCompoletionAction, QuestID, Context, ENetworkActionType::ServerOnly);

				if (QuestData->bAutoClaimReward)
				{
					this->Internal_ClaimQuestReward(QuestID);
				}
			}
		}
	));
}

void UQuestComponent::OnQuestProgressUpdated(UQuestObject* UpdatedQuest, const FGameplayTag& ObjID, int32 NewValue)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for OnQuestProgressUpdated. Need Authority."));
		return;
	}

	if (!ensureMsgf(UpdatedQuest && UpdatedQuest->GetQuestID().IsValid(), TEXT("QuestObject Internal Error")))
	{
		return;
	}

	const FGameplayTag& QuestID = UpdatedQuest->GetQuestID();
	QuestProgressList.UpdateProgressData(QuestID, ObjID, NewValue);
}

void UQuestComponent::RestoreActiveQuest()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Quests Are Now Active"));

	for (const FQuestProgressData& QuestData : QuestProgressList)
	{
		if (QuestData.IsActiveState())
		{
			LoadAndActivateQuest(QuestData.GetQuestID());
		}
	}
}

void UQuestComponent::StopActiveQuests()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Qeusts Are Now DeActivate"));

	for (TObjectPtr<UQuestObject> CurrentQuest : ActiveQuests)
	{
		DeactivateAndDestroyQuest(CurrentQuest->GetQuestID());
	}
}
#pragma endregion


// ----------------------------------------------------------------
// Legacy Support & DTO Builders
// ----------------------------------------------------------------
# pragma region Legacy Support
/*
void UQuestComponent::NotifyQuestUpdate(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] NotifyQuestUpdate: [%s] Id is Updated..."), *QuestID.GetTagName().ToString());

	// 델리게이트에 구독자가 있는지 확인
	if (OnQuestEntryUpdatedDelegate.IsBound())
	{
		// DTO 빌더를 호출하여 최신 DTO 생성
		FQuestLogEntry UpdatedEntry;
		if (BuildQuestLogEntry(QuestID, UpdatedEntry))
		{
			OnQuestEntryUpdatedDelegate.Broadcast(UpdatedEntry);
		}
	}

	// TODO: 별개 책임. 이동 예정
	const UQuestObject* CurrentObject = FindActiveQuest(QuestID);
	if (CurrentObject)
	{
		if (const FQuestProgressData* CurrentProgressData = QuestProgressList.Find(QuestID))
		{
			if (CurrentObject->CheckQuestCompletion() ||
				CurrentProgressData->GetProgressType() == EQuestProgress::Completed_PendingTurnIn)
			{
				DeactivateAndDestroyQuest(QuestID);
			}
		}
	}
}*/

/*
bool UQuestComponent::BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const
{
	if (GetOwnerRole() == ENetRole::ROLE_Authority)
	{
		return false;
	}

	if (!QuestID.IsValid())
	{
		return false;
	}

	const FQuestProgressData* ProgressData = QuestProgressList.Find(QuestID);
	if (!ProgressData)
	{
		return false;
	}

	const FQuestTableRow* QuestMetadata = QuestMetadataCache.Find(QuestID);
	if (QuestMetadata)
	{
		OutEntry.QuestID = QuestID;
		OutEntry.Title = QuestMetadata->QuestName;
		OutEntry.Description = QuestMetadata->Description;
		OutEntry.CurrentState = ProgressData->GetProgressType();

		if (ProgressData->IsActiveState())
		{
			const TArray<TObjectPtr<UQuestObjectiveConfig>>& ObjectConfigs = FindActiveQuest(QuestID)->GetQuestDefinition()->ObjectConfigs;
			OutEntry.FormattedObjectives.Empty(ObjectConfigs.Num());

			for (const UQuestObjectiveConfig* ObjConfig : ObjectConfigs)
			{
				if (ObjConfig)
				{
					FText FormattedText = ObjConfig->GetFormattedObjectiveText(*ProgressData);
					OutEntry.FormattedObjectives.Add(FormattedText);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] BuildQuestLogEntry: QuestID [%s] DA is not found!"), *QuestID.GetTagName().ToString());
		return false; // 실패
	}
	return true;
}*/
#pragma endregion


// ----------------------------------------------------------------
// Debug Tools
// ----------------------------------------------------------------
#pragma region Debug Tools
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
void UQuestComponent::Cheat_ForceCompleteQuest(const FString& QuestID)
{
	UQuestObject* QuestRef = ActiveQuests.FindRef(FGameplayTag::RequestGameplayTag(*QuestID));
	if (!QuestRef)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cheat_ForceCompleteQuest: QuestID [%s] is not valid. Need to call Cheat_SetupNewGameQuest First"), *QuestID);
		return;
	}

	QuestRef->ForceCompleteQuest();
}

void UQuestComponent::Cheat_ForceCompleteQuestObj(const FString& QuestID, const FString& ObjectiveID)
{
	UQuestObject* QuestRef = ActiveQuests.FindRef(FGameplayTag::RequestGameplayTag(*QuestID));
	if (!QuestRef)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cheat_ForceCompleteQuestObj: QuestID [%s] is not valid. Need to call Cheat_SetupNewGameQuest First"), *QuestID);
		return;
	}

	QuestRef->ForceCompleteQuestObj(FGameplayTag::RequestGameplayTag(*ObjectiveID));
}

void UQuestComponent::Console_ForceCompleteQuest(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("Usage: Quest.ForceComplete <QuestID>"));
		return;
	}
	// 인자를 받아서 실제 치트 함수로 전달
	Cheat_ForceCompleteQuest(Args[0]);
}

void UQuestComponent::Console_ForceCompleteQuestObj(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("Usage: Quest.ForceCompleteObj <QuestID> <ObjectiveID>"));
		return;
	}
	// 인자를 받아서 실제 치트 함수로 전달
	Cheat_ForceCompleteQuestObj(Args[0], Args[1]);
}
#endif
#pragma endregion