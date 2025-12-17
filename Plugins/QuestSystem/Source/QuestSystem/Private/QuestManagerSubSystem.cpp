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

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif
#include "Reward/QuestReward.h"

// ============ 공용 함수 ===============


// ==================================
// 기본 초기화
// ==================================



void UQuestManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
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



void UQuestManagerSubSystem::Deinitialize()
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

// ==================================
// UI를 위한 DTO 제공 API
// ==================================



TArray<FQuestLogEntry> UQuestManagerSubSystem::GetQuestLogEntries() const
{
	TArray<FQuestLogEntry> LogEntries;
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] GetQuestLogEntries: [%d] in PlayerQuestHistory..."), PlayerQuestHistory.Num());
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


// ==================================
// 직렬화된 저장 관련
// ==================================



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



// ==================================
// 캡슐화를 위한 진행도 질의 함수
// ==================================



FQuestProgressData* UQuestManagerSubSystem::QueryProgressDataForQuestID(const FGameplayTag& QuestID)
{
	if (!QuestID.IsValid())
	{
		return nullptr;
	}

	return PlayerQuestHistory.Find(QuestID);
}



void UQuestManagerSubSystem::SetupNewGameQuests()
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
	for (const TPair<FGameplayTag, TObjectPtr<UDA_QuestBase>>& Pair : ActiveQuestDACaches)
	{
		UDA_QuestBase* QuestDef = Pair.Value;
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



void UQuestManagerSubSystem::AcceptQuest(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] AcceptQuest: [%s] Id is Accepting Now..."), *QuestID.GetTagName().ToString());
	if (ActiveQuests.Contains(QuestID))
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("Quest is already active!"));
		return;
	}

	// 퀘스트를 수락하고 진행 상태로 변경합니다.
	PlayerQuestHistory.Emplace(QuestID, EQuestProgress::InProgress);

	LoadAndActivateQuest(QuestID);
}


void UQuestManagerSubSystem::ClaimQuestReward(const FGameplayTag& QuestID)
{
	// 퀘스트가 보상 대기 상태인지 검사
	FQuestProgressData* CurrentQuestData = PlayerQuestHistory.Find(QuestID);
	if (!CurrentQuestData || CurrentQuestData->ProgressType != EQuestProgress::Completed_PendingTurnIn)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] ClaimQuestReward: [%s] ID has not validate data"), *QuestID.GetTagName().ToString());
		return;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] ClaimQuestReward: Complete Quest for [%s] ID"), *QuestID.GetTagName().ToString());
	CurrentQuestData->ProgressType = EQuestProgress::Complete_Final;

	FQuestLogEntry UpdatedEntry;
	if (BuildQuestLogEntry(QuestID, UpdatedEntry))
	{
		OnQuestEntryUpdatedDelegate.Broadcast(UpdatedEntry);
	}

	// 퀘스트 보상 제공
	GiveReward(QuestID);

	// 후행 퀘스트 상태 전이 확인
	TryUnlockNextQuests(QuestID);
}

void UQuestManagerSubSystem::GiveReward(const FGameplayTag& QuestID)
{
	UDA_QuestBase* QuestDef = ActiveQuestDACaches.FindRef(QuestID);
	if (!QuestDef)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] GiveReward: Cannot find DA for [%s]"), *QuestID.ToString());
		return;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Giving Rewards for Quest [%s]..."), *QuestID.ToString());

	// 정의된 모든 보상 객체의 GiveReward 실행
	for (UQuestReward* Reward : QuestDef->QuestRewards)
	{
		if (Reward)
		{
			// Subsystem은 UObject이므로 this를 컨텍스트로 전달
			Reward->GiveReward(this);
		}
	}
}

void UQuestManagerSubSystem::TryUnlockNextQuests(const FGameplayTag& QuestID)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] TryUnlockNextQuests: Unlock Quest Next of [%s] ID"), *QuestID.GetTagName().ToString());

	UDA_QuestBase* CurrentQuestDEf = ActiveQuestDACaches.FindRef(QuestID);
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

			UDA_QuestBase* NextQuestObjPtr = ActiveQuestDACaches.FindRef(NextID);
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



void UQuestManagerSubSystem::NotifyQuestUpdate(const FGameplayTag& QuestID)
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

	UQuestObject* CurrentObject = ActiveQuests.FindRef(QuestID);
	if (CurrentObject)
	{
		const FQuestProgressData& CurrentProgress = PlayerQuestHistory.FindRef(QuestID);
		if (CurrentObject->CheckQuestCompletion() ||
			CurrentProgress.ProgressType == EQuestProgress::Completed_PendingTurnIn)
		{
			DeactivateAndDestroyQuest(QuestID);
		}
	}
}



// ==================================
// 퀘스트 전체적인 활성화 제어 관련
// ==================================



void UQuestManagerSubSystem::StartActiveQuests()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Quests Are Now Active"));

	for (auto& QuestPair : PlayerQuestHistory)
	{
		FQuestProgressData& ProgressData = QuestPair.Value;
		if (ProgressData.ProgressType == EQuestProgress::InProgress)
		{
			LoadAndActivateQuest(QuestPair.Key);
		}
	}
}

void UQuestManagerSubSystem::StopActiveQuests()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Qeusts Are Now DeActivate"));

	for (TPair<FGameplayTag, TObjectPtr<UQuestObject>> CurrentQuest : ActiveQuests)
	{
		DeactivateAndDestroyQuest(CurrentQuest.Key);
	}
}



// ==================================
// 데이터 로드 관련
// ==================================



void UQuestManagerSubSystem::StartAsyncLoadData()
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
}

void UQuestManagerSubSystem::OnQuestDataLoaded()
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("All QuestData assets are now loaded. Caching..."));

	// 로드 요청했던 목록을 다시 가져오거나, 멤버 변수로 저장해둔 목록을 순회합니다.
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIdList;
	AssetManager.GetPrimaryAssetIdList(FName("QuestData"), AssetIdList);

	for (const FPrimaryAssetId& AssetId : AssetIdList)
	{
		// 로드가 완료되었으므로, GetPrimaryAssetObject는 즉시 유효한 UObject*를 반환합니다.
		UDA_QuestBase* QuestData = Cast<UDA_QuestBase>(AssetManager.GetPrimaryAssetObject(AssetId));

		if (QuestData)
		{
			// FName(ID)을 키로 TMap에 캐싱합니다.
			ActiveQuestDACaches.FindOrAdd(QuestData->QuestID, QuestData);
		}
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("Caching complete. %d quests loaded."), ActiveQuestDACaches.Num());

	StartActiveQuests();
}



// ==================================
// 퀘스트 런타임 객체 관리
// ==================================



void UQuestManagerSubSystem::LoadAndActivateQuest(const FGameplayTag& QuestID)
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

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cannot activate quest [%s], World is NULL."), *QuestDef->GetPrimaryAssetId().PrimaryAssetName.ToString());
		return;
	}

	// 새로운 UAEQuestObject를 생성하고 초기화
	UQuestObject* NewQuestObject = NewObject<UQuestObject>(this);
	NewQuestObject->Initialize(QuestDef, this);

	NewQuestObject->OnQuestObjectChangedDelegate.BindUObject(this, &UQuestManagerSubSystem::NotifyQuestUpdate);
	NewQuestObject->OnRequestWorldTasksDelegate.BindUObject(this, &UQuestManagerSubSystem::OnQuestRequestingWorldTasks);

	NewQuestObject->Activate(World);
	ActiveQuests.FindOrAdd(QuestID, NewQuestObject);

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] ActivateQuestObject: [%s] Id is activated completely"), *QuestDef->GetPrimaryAssetId().PrimaryAssetName.ToString());
}

void UQuestManagerSubSystem::DeactivateAndDestroyQuest(const FGameplayTag& QuestID)
{
	// 퀘스트를 비활성화합니다.
	UQuestObject* QuestObj = ActiveQuests.FindRef(QuestID);
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] DeactivateAndDestroyQuest: Object [%s] is deactivated"), *QuestObj->GetFName().ToString());

	if (QuestObj)
	{
		QuestObj->OnQuestObjectChangedDelegate.Unbind();
		QuestObj->OnRequestWorldTasksDelegate.Unbind();
	}
	ActiveQuests.Remove(QuestID);
}



// ==================================
// 태스크 객체 관련
// ==================================



void UQuestManagerSubSystem::OnQuestRequestingWorldTasks(const TArray<TObjectPtr<UQuestTask>>& TasksToExecute)
{
	OnQuestTaskBubbleUpDelegate.ExecuteIfBound(TasksToExecute);
}



// ==================================
// UI 용 DTO 관련
// ==================================



bool UQuestManagerSubSystem::BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const
{
	const FQuestProgressData ProgressData = PlayerQuestHistory.FindRef(QuestID);

	if (!QuestID.IsValid()) 
	{
		return false;
	}

	UDA_QuestBase* QuestDef = ActiveQuestDACaches.FindRef(QuestID);
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] BuildQuestLogEntry: Making entry in questdef [%s]"), *QuestDef->GetFName().ToString());
	if (QuestDef)
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



// ==================================
// DEVELOPMENT_ONLY
// ==================================


#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

void UQuestManagerSubSystem::Cheat_SetupNewGameQuests()
{
	SetupNewGameQuests();
}

void UQuestManagerSubSystem::Cheat_ForceCompleteQuest(const FString& QuestID)
{
	UQuestObject* QuestRef = ActiveQuests.FindRef(FGameplayTag::RequestGameplayTag(*QuestID));
	if (!QuestRef)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cheat_ForceCompleteQuest: QuestID [%s] is not valid. Need to call Cheat_SetupNewGameQuest First"), *QuestID);
		return;
	}

	QuestRef->ForceCompleteQuest();
}

void UQuestManagerSubSystem::Cheat_ForceCompleteQuestObj(const FString& QuestID, const FString& ObjectiveID)
{
	UQuestObject* QuestRef = ActiveQuests.FindRef(FGameplayTag::RequestGameplayTag(*QuestID));
	if (!QuestRef)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] Cheat_ForceCompleteQuestObj: QuestID [%s] is not valid. Need to call Cheat_SetupNewGameQuest First"), *QuestID);
		return;
	}

	QuestRef->ForceCompleteQuestObj(FGameplayTag::RequestGameplayTag(*ObjectiveID));
}

void UQuestManagerSubSystem::Console_ForceCompleteQuest(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("Usage: Quest.ForceComplete <QuestID>"));
		return;
	}
	// 인자를 받아서 실제 치트 함수로 전달
	Cheat_ForceCompleteQuest(Args[0]);
}

void UQuestManagerSubSystem::Console_ForceCompleteQuestObj(const TArray<FString>& Args)
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