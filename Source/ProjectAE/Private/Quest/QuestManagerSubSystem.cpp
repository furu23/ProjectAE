// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestManagerSubSystem.h"
#include "Core/AEGloabalHelper.h"
#include "Quest/AEQuestTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Quest/Data/DA_QuestBase.h"
#include "Quest/AEQuestObject.h"

DEFINE_LOG_CATEGORY(LogQuestSystem);

void UQuestManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	PlayerQuestHistory.Empty();

	FQuestProgressData QuestData;
	QuestData.ProgressType = EQuestProgress::CanAccept;
	PlayerQuestHistory.Add(FGameplayTag::RequestGameplayTag("Quest.Id.Interact.GetBox"), QuestData);
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
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] In-Raid Level is started now"));

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
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] In-Raid Level is ended now"));

	for (UAEQuestObject* QuestObject : ActiveQuests)
	{
		DeactivateAndDestroyQuest(QuestObject);
	}
	ActiveQuests.Empty();
}

void UQuestManagerSubSystem::LoadAndActivateQuest(FGameplayTag QuestID, FQuestProgressData* ProgressData)
{
	if (!QuestID.IsValid())
	{
		UE_LOG(LogQuestSystem, Warning, TEXT("[QuestSys] LoadAndActivateQuest: QuestID is invalid."));
		return;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	FPrimaryAssetId QuestAssetId(FName("QuestData"), QuestID.GetTagName()); // "QuestData"는 프로젝트 세팅과 동일해야 합니다.

	// 1. 이미 로드되었는지 확인합니다 (가장 빠른 경로)
	UDA_QuestBase* QuestDef = AssetManager.GetPrimaryAssetObject<UDA_QuestBase>(QuestAssetId);
	if (QuestDef)
	{
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] LoadAndActivateQuest: [%s] was already loaded. Activating."), *QuestID.GetTagName().ToString());
		// 이미 있으니 바로 활성화 함수 호출
		ActivateQuestObject(QuestDef, ProgressData);
		return;
	}

	// 2. 로드되지 않았다면 비동기 로드를 요청합니다.
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] LoadAndActivateQuest: [%s] not loaded. Requesting async load..."), *QuestID.GetTagName().ToString());

	// 로드가 완료되면 실행될 람다(Lambda) 함수를 정의합니다.
	// [this, QuestAssetId, ProgressData, QuestID] : 람다 안에서 사용할 변수들을 캡처합니다.
	FStreamableDelegate DoneDelegate = FStreamableDelegate::CreateLambda(
		[this, QuestAssetId, ProgressData, QuestID]()
		{
			// --- 이 블록은 로드가 완료된 '미래' 시점에 실행됩니다 ---

			UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] Async load complete for: [%s]"), *QuestID.GetTagName().ToString());

			// 로드가 완료되었으니 이제 GetPrimaryAssetObject는 성공합니다.
			UAssetManager& AssetManagerRef = UAssetManager::Get();
			UDA_QuestBase* LoadedQuestDef = AssetManagerRef.GetPrimaryAssetObject<UDA_QuestBase>(QuestAssetId);

			if (LoadedQuestDef)
			{
				// 로드 성공! 활성화 함수 호출
				ActivateQuestObject(LoadedQuestDef, ProgressData);
			}
			else
			{
				// ID는 맞았는데 로드에 실패한 경우 (에셋이 깨졌거나 경로가 잘못된 경우)
				UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] FAILED to get quest object for [%s] even after async load! Asset may be invalid."), *QuestID.GetTagName().ToString());
			}
		}
	);

	// 3. 에셋 매니저에게 로드를 요청합니다.
	AssetManager.LoadPrimaryAsset(QuestAssetId, TArray<FName>(), DoneDelegate);
}

void UQuestManagerSubSystem::ActivateQuestObject(UDA_QuestBase* QuestDef, FQuestProgressData* ProgressData)
{
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

	// --- 기존 로직 시작 ---

	// 새로운 UAEQuestObject를 생성하고 초기화합니다.
	UAEQuestObject* NewQuestObject = NewObject<UAEQuestObject>(this);
	NewQuestObject->Initialize(QuestDef, ProgressData, this);

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

/*
void UQuestManagerSubSystem::LoadAndActivateQuest(FGameplayTag QuestID, FQuestProgressData* ProgressData)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] LoadAndActivateQuest: [%s] Id is in Loading"), *QuestID.GetTagName().ToString());


	// UAssetManager를 사용하여 퀘스트 정의를 로드합니다.
	UAssetManager& AssetManager = UAssetManager::Get();

	FPrimaryAssetId QuestAssetId(FName("QuestData"), QuestID.GetTagName());
	UDA_QuestBase* QuestDef = AssetManager.Lo

	// 퀘스트 정의가 유효한지 확인합니다.
	if (QuestDef)
	{
		// 새로운 UAEQuestObject를 생성하고 초기화합니다.
		UAEQuestObject* NewQuestObject = NewObject<UAEQuestObject>(this);
		NewQuestObject->Initialize(QuestDef, ProgressData, this);
		// 델리게이트를 바인딩합니다.
		NewQuestObject->OnQuestObjectChangedDelegate.BindUObject(this, &UQuestManagerSubSystem::NotifyQuestUpdate);
		// NewQuestObject->OnRequestWorldTasksDelegate.BindUObject(this, &UQuestManagerSubSystem::OnQuestRequestingWorldTasks);
		// 퀘스트를 활성화합니다.
		NewQuestObject->Activate(this->GetWorld());

		// 활성화된 퀘스트 목록에 추가합니다.
		ActiveQuests.Add(NewQuestObject);

		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] LoadAndActivateQuest: [%s] Id is loaded completely"), *QuestID.GetTagName().ToString());
	}
}*/

void UQuestManagerSubSystem::DeactivateAndDestroyQuest(UAEQuestObject* QuestObject)
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

	UE_LOG(LogTemp, Log, TEXT("진행도 구조체는 현재 미구현입니다."));

	return true; // (모든 로직이 성공했다고 가정)
}