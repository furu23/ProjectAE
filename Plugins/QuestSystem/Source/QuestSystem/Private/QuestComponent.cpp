#include "QuestComponent.h"
#include "QuestSystem.h"
#include "QuestTypes.h"
#include "QuestObject.h"
#include "Data/QuestObjectConfig.h"
#include "Objectives/QuestObjectiveConfig.h"

#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif
#include "Reward/QuestReward.h"
#include "Action/QuestAction.h"



// ==================================
// 기본 초기화
// ==================================


UQuestComponent::UQuestComponent()
{

}


void UQuestComponent::BeginPlay(FSubsystemCollectionBase& Collection)
{
	PlayerQuestHistory.Empty();

#if !UE_BUILD_SHIPPING

	if (this->GetLocalPlayer() != this->GetWorld()->GetGameInstance()->GetFirstGamePlayer())
	{
		return;
	}

	IConsoleManager& ConsoleMgr = IConsoleManager::Get();

	// 1. 퀘스트 리셋 (인자 없음)
	ConsoleCommands.Add(ConsoleMgr.RegisterConsoleCommand(
		TEXT("Quest.Reset"),
		TEXT("Resets all quest progress (New Game Setup). Usage: Quest.Reset"),
		FConsoleCommandDelegate::CreateUObject(this, &UQuestManagerSubSystem::Cheat_SetupNewGameQuests),
		ECVF_Cheat
	));

	// 2. 퀘스트 강제 완료 (인자 1개)
	ConsoleCommands.Add(ConsoleMgr.RegisterConsoleCommand(
		TEXT("Quest.ForceComplete"),
		TEXT("Force completes a quest. Usage: Quest.ForceComplete <QuestID>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UQuestManagerSubSystem::Console_ForceCompleteQuest),
		ECVF_Cheat
	));

	// 3. 목표 강제 완료 (인자 2개)
	ConsoleCommands.Add(ConsoleMgr.RegisterConsoleCommand(
		TEXT("Quest.ForceCompleteObj"),
		TEXT("Force completes a specific objective. Usage: Quest.ForceCompleteObj <QuestID> <ObjID>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UQuestManagerSubSystem::Console_ForceCompleteQuestObj),
		ECVF_Cheat
	));

	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] Debug Console Commands Registered."));
#endif
}



void UQuestComponent::BeginPlay()
{
	
}

void UQuestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

}

void UQuestComponent::Deinitialize()
{
#if !UE_BUILD_SHIPPING
	for (IConsoleObject* Cmd : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Cmd);
	}
	ConsoleCommands.Empty();
#endif

	Super::Deinitialize();
}

void UQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

}

// ==================================
// UI를 위한 DTO 제공 API
// ==================================



TArray<FQuestLogEntry> UQuestComponent::GetQuestLogEntries() const
{
	TArray<FQuestLogEntry> LogEntries;

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] GetQuestLogEntries: [%d] in PlayerQuestHistory..."), PlayerQuestHistory.Num());
	for (const FQuestProgressData ProgressData : PlayerQuestHistory)
	{
		FQuestLogEntry Entry;
		if (BuildQuestLogEntry(ProgressData.QuestID, Entry))
		{
			LogEntries.Add(Entry);
		}
	}
	return LogEntries;
}


const UQuestObject* UQuestComponent::FindActiveQuest(const FGameplayTag& QuestID) const
{
	auto* ResultPtr = ActiveQuests.FindByPredicate([&](const auto& Quest)
		{
			return Quest && Quest->GetQuestID() == QuestID;
		});

	return ResultPtr ? ResultPtr->Get() : nullptr;
}


bool UQuestComponent::HasActiveQuest(const FGameplayTag& QuestID) const
{
	return ActiveQuests.ContainsByPredicate([&](const UQuestObject* Quest)
		{
			return Quest && Quest->GetQuestID() == QuestID;
		});
}

// ==================================
// 직렬화된 저장 관련
// ==================================



void UQuestComponent::GetSaveData(TArray<uint8>& OutData)
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
}


// ==================================
// 캡슐화를 위한 진행도 질의 함수
// ==================================



const FQuestProgressData* UQuestComponent::QueryProgressDataForQuestID(const FGameplayTag& QuestID) const
{
	if (!QuestID.IsValid())
	{
		return nullptr;
	}

	return PlayerQuestHistory.Find(QuestID);
}



void UQuestComponent::SetupNewGameQuests()
{
	if (PlayerQuestHistory.Num() > 0)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("SetupNewGameQuests called but History is not empty!"));
		return;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Setting up new game quests..."));

	// 데이터 초기화
	PlayerQuestHistory.Empty();

	// 캐시된 모든 퀘스트 데이터 에셋을 순회
	for (const TPair<FGameplayTag, TObjectPtr<UQuestObjectConfig>>& Pair : ActiveQuestDACaches)
	{
		UQuestObjectConfig* QuestDef = Pair.Value;
		if (QuestDef && QuestDef->bIsStartingQuest)
		{
			// 해당 퀘스트를 CanAccept 상태로 등록
			PlayerQuestHistory.FindOrAdd(QuestDef->QuestID).ProgressType = EQuestProgress::CanAccept;

			UE_LOG(LogQuestSystem, Verbose, TEXT(" - Initial Quest Set: [%s]"), *QuestDef->QuestID.ToString());
		}
		else if (QuestDef)
		{
			PlayerQuestHistory.FindOrAdd(QuestDef->QuestID).ProgressType = EQuestProgress::NotStarted;
		}
	}
}



// ============ 내부 함수 ==============


// ==================================
// 퀘스트 플로우 관련
// ==================================



void UQuestComponent::AcceptQuest(const FGameplayTag& QuestID)
{
	if (!QuestID.IsValid()) return;

	const FQuestProgressData* ExistingData = PlayerQuestHistory.Find(QuestID);
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

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] AcceptQuest: [%s] Id is Accepting Now..."), *QuestID.GetTagName().ToString());
	if (HasActiveQuest(QuestID))
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("Quest is already active!"));
		return;
	}

	// 퀘스트를 수락하고 진행 상태로 변경합니다.
	PlayerQuestHistory.UpdateProgressData(QuestID, EQuestProgress::InProgress);

	LoadAndActivateQuest(QuestID);
}


void UQuestComponent::ClaimQuestReward(const FGameplayTag& QuestID)
{
	// 퀘스트가 보상 대기 상태인지 검사
	const FQuestProgressData* CurrentQuestData = PlayerQuestHistory.Find(QuestID);
	if (!CurrentQuestData || CurrentQuestData->GetProgressType() != EQuestProgress::Completed_PendingTurnIn)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] ClaimQuestReward: [%s] ID has not validate data"), *QuestID.GetTagName().ToString());
		return;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] ClaimQuestReward: Complete Quest for [%s] ID"), *QuestID.GetTagName().ToString());
	PlayerQuestHistory.UpdateProgressData(QuestID, EQuestProgress::Complete_Final);

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

void UQuestComponent::GiveReward(const FGameplayTag& QuestID)
{
	const UQuestObject* CurrentQuest = FindActiveQuest(QuestID);
	if (!CurrentQuest)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] GiveReward: Cannot find Active QuestObject for [%s]"), *QuestID.ToString());
		return;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Giving Rewards for Quest [%s]..."), *QuestID.ToString());

	// 퀘스트 액션 실행
	if (const UQuestObjectConfig* CurrentConfig = CurrentQuest->GetQuestDefinition())
	{
		for (UQuestAction* Quests : CurrentConfig->QuestClaimRewardAction)
		{
			Quests->ExecuteAction(this, QuestID);
		}
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

			FQuestProgressData* ExistingData = PlayerQuestHistory.Find(NextID);
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
				const FQuestProgressData* PrereqHistory = PlayerQuestHistory.Find(PrereqID);

				// 기록이 아예 없거나, 최종 완료 상태가 아니라면 탈락
				if (!PrereqHistory || PrereqHistory->ProgressType != EQuestProgress::Complete_Final)
				{
					bAllPrereqsMet = false;
					break;
				}
			}

			if (bAllPrereqsMet)
			{
				FQuestProgressData* NewQuestData = PlayerQuestHistory.Find(NextID);
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



// ==================================
// 하위클래스 전용 상태 알림 함수
// ==================================



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
		if (const FQuestProgressData* CurrentProgressData = PlayerQuestHistory.Find(QuestID))
		{
			if (CurrentObject->CheckQuestCompletion() ||
				CurrentProgressData->GetProgressType() == EQuestProgress::Completed_PendingTurnIn)
			{
				DeactivateAndDestroyQuest(QuestID);
			}
		}
	}
}



// ==================================
// 퀘스트 전체적인 활성화 제어 관련
// ==================================



void UQuestComponent::StartActiveQuests()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Quests Are Now Active"));

	for (const FQuestProgressData& QuestData : PlayerQuestHistory)
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



// ==================================
// 데이터 로드 관련
// ==================================



void UQuestComponent::LoadAndActivateQuest(const FGameplayTag& QuestID)
{
	UAssetManager& AssetManager = UAssetManager::Get();

	FPrimaryAssetId AssetID(FName("QuestData"), QuestID.GetTagName());
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetID);
	if (!AssetPath.IsValid())
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] StartQuestDataLoad: Invalid AssetID for [%s]. Check AssetManager settings."), *QuestID.ToString());
		return;
	}

	FStreamableDelegate OnLoadCompleteDelegate = FStreamableDelegate::CreateWeakLambda(this, [this, QuestID]()
		{
			this->OnQuestDataLoaded(QuestID);
		});
	
	TSharedPtr<FStreamableHandle> LoadHandle = AssetManager.LoadPrimaryAsset(AssetID, TArray<FName>(), OnLoadCompleteDelegate);

	if (LoadHandle.IsValid())
	{
		LoadHandles.Add(QuestID, LoadHandle);
	}
}

void UQuestComponent::OnQuestDataLoaded(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("All QuestData assets are now loaded. Caching..."));

	if (LoadHandles.Contains(QuestID))
	{
		LoadHandles.Remove(QuestID);
	}

	StartActivateQuest(QuestID);
}



// ==================================
// 퀘스트 런타임 객체 관리
// ==================================



void UQuestComponent::StartActivateQuest(const FGameplayTag& QuestID)
{
	if (!QuestID.IsValid())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("[QuestSys] LoadAndActivateQuest: QuestID is invalid."));
		return;
	}

	// TODO: IsContainActivateQuest 로 변경
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
	UQuestObject* NewQuestObject = NewObject<UQuestObject>(this);
	NewQuestObject->Initialize(QuestData, this);

	NewQuestObject->OnQuestObjectChangedDelegate.BindUObject(this, &UQuestComponent::NotifyQuestUpdate);
	NewQuestObject->OnRequestWorldTasksDelegate.BindUObject(this, &UQuestComponent::OnQuestRequestingWorldTasks);

	NewQuestObject->Activate();

	ActiveQuests.Add(NewQuestObject);

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] StartActivateQuest: Quest [%s] is activated completely"), *QuestID.ToString());
}

void UQuestComponent::DeactivateAndDestroyQuest(const FGameplayTag& QuestID)
{
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


// ==================================
// UI 용 DTO 관련
// ==================================



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

	const FQuestProgressData* ProgressData = PlayerQuestHistory.Find(QuestID);
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
}





// ==================================
// DEVELOPMENT_ONLY
// ==================================


#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

void UQuestComponent::Cheat_SetupNewGameQuests()
{
	SetupNewGameQuests();
}

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