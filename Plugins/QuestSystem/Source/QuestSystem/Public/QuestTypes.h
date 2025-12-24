#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "QuestSystem.h"
#include "QuestTypes.generated.h"

// 현재 퀘스트 데이터 구조의 버전입니다.
// 구조체가 변경될 때마다 이 숫자를 1씩 증가시키고, Migration 로직을 추가해야 합니다.
static constexpr int32 QUEST_DATA_CURRENT_VERSION = 1;


UENUM(BlueprintType)
enum class EQuestProgress : uint8
{
	None						UMETA(Hidden),
	NotStarted					UMETA(ToolTip = "아직 수령하지 않은 상태입니다."),
	CanAccept					UMETA(ToolTip = "수령할 수 있는 상태입니다."),
	InProgress					UMETA(ToolTip = "진행 중인 상태입니다."),
	Completed_PendingTurnIn		UMETA(DisplayName = "WaitForReward", ToolTip = "In-Raid 레벨에서 의뢰를 완수하고 보상을 기다리는 상태입니다."),
	Complete_Final				UMETA(DisplayName = "Completed", ToolTip = "완료된 상태입니다.")
};


UENUM(BlueprintType)
enum class EQuestType : uint8
{
	None		UMETA(Hidden),
	Main		UMETA(ToolTip = "메인 퀘스트입니다."),
	Side		UMETA(ToolTip = "서브 퀘스트입니다."),
	Daily		UMETA(ToolTip = "데일리 퀘스트입니다."),
	Event		UMETA(ToolTip = "이벤트 퀘스트입니다.")
};


USTRUCT(BlueprintType)
struct FQuestTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="System")
    FGameplayTagContainer PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="System")
    EQuestType QuestType; // Main, Side, Daily...

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dev")
	FText DevComment;
};


USTRUCT(BlueprintType)
struct FQuestObjectiveData
{
    GENERATED_BODY()

public:
	FQuestObjectiveData() {}

	explicit FQuestObjectiveData(const FGameplayTag& ObjectiveID) : ObjectiveID(ObjectiveID) {}
	explicit FQuestObjectiveData(const FGameplayTag& ObjectiveID, int32 Count) : ObjectiveID(ObjectiveID), Count(Count) {}


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
    FGameplayTag ObjectiveID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
    int32 Count = 0;

    bool operator==(const FGameplayTag& Tag) const { return ObjectiveID == Tag; }
};

USTRUCT(BlueprintType)
struct FQuestProgressData : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FQuestProgressData() {}

	explicit FQuestProgressData(const FGameplayTag& QuestID) : QuestID(QuestID) {}
	explicit FQuestProgressData(const FGameplayTag& QuestID, EQuestProgress ProgressType) : QuestID(QuestID), ProgressType(ProgressType) {}

private:
	// **** 실제 퀘스트 진행 상태 프로퍼티 ****

	// 퀘스트 고유 ID
	UPROPERTY(SaveGame)
	FGameplayTag QuestID = FGameplayTag::EmptyTag;
	
	// 현재 진행도의 단계
	UPROPERTY(SaveGame)
	EQuestProgress ProgressType = EQuestProgress::None;

	// 진행도의 실제 진행상황
	UPROPERTY(SaveGame)
	TArray<FQuestObjectiveData> Objectives;



	// **** 버전 관리용 프로퍼티 ****

	UPROPERTY(SaveGame)
    int32 Version = QUEST_DATA_CURRENT_VERSION;


public:
	// **** 직렬화 마이그레이션 함수 ****

	/**
	 * @brief 이 함수는 저장된 데이터의 버전을 현재 버전으로 마이그레이션합니다.
	 * @note 구조체가 변경될 때마다 이 함수에 마이그레이션 로직을 추가해야 합니다.
	 * 
	 * @return true 데이터가 변경되어 마이그레이션이 수행된 경우
	 * @return false 데이터가 이미 최신 버전인 경우
	 */
	bool MigrateToLatest()
	{
		if (Version == QUEST_DATA_CURRENT_VERSION) return false;

		// 예: if (Version < 1) { ... 구버전 데이터 변환 ... }

		Version = QUEST_DATA_CURRENT_VERSION;
		return true;
	}

	// **** Objective 추가 함수 ****

	void AddObjective(const FGameplayTag& ObjID, int32 InitialCount = 0)
	{
		if (Objectives.FindByKey(ObjID))
		{
			UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Add in Objective ID [%s]"), *ObjID.ToString());
			return;
		}
		
		Objectives.Add(FQuestObjectiveData(ObjID, InitialCount));
	}


	// **** 편의 함수 모음 ****

	// ObjectiveID에 해당되는 FQuestObjectiveData의 값을 질의하고 받습니다
	FORCEINLINE const FQuestObjectiveData* FindObjective(const FGameplayTag& ObjID) const
	{
		return Objectives.FindByKey(ObjID);
	}

	FORCEINLINE int32 ObjNum() const 
	{
		return Objectives.Num();
	}

	FORCEINLINE bool IsObjEmpty() const
	{ 
		return Objectives.Num() == 0;
	}

	FORCEINLINE bool IsActiveState() const
	{
		return ProgressType == EQuestProgress::InProgress || ProgressType == EQuestProgress::Completed_PendingTurnIn;
	}

	FORCEINLINE bool IsCompleteState() const
	{
		return ProgressType == EQuestProgress::Complete_Final;
	}


	// **** 게터 함수 ****

	FORCEINLINE const FGameplayTag& GetQuestID() const
	{
		return QuestID;
	}

	FORCEINLINE const TArray<FQuestObjectiveData>& GetObjectives() const
	{
		return Objectives;
	}

	FORCEINLINE EQuestProgress GetProgressType() const
	{
		return ProgressType;
	}

	FORCEINLINE int32 GetProgressDataVersion() const
	{
		return Version;
	}


	// **** 세터 함수 ****

	bool UpdateObjective(const FGameplayTag& ObjID, int32 NewValue)
	{
		if (!ObjID.IsValid()) return false;

		FQuestObjectiveData* FindObj = Objectives.FindByKey(ObjID);
		if (!FindObj)
		{
			UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] InValid Access for ProgressData Update in Objective ID [%s]"), *ObjID.ToString());
			return false;
		}

		FindObj->Count = NewValue;
		return true;
	}

	FORCEINLINE void UpdateProgress(EQuestProgress InProgressType)
	{
		ProgressType = InProgressType;
	}

	FORCEINLINE void UpdateAllObjectives(const TArray<FQuestObjectiveData>& InObjectives)
	{
		Objectives = InObjectives;
	}


	// **** 연산자 오버로드 ****

	// 비교 연산자 오버로드
	bool operator==(const FGameplayTag& Tag) const { return QuestID == Tag; }
};




USTRUCT(BlueprintType)
struct FQuestFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	// 선형 검색을 사용할 임계값 (변경 자유)
	static constexpr int32 LinearSearchThreshold = 50;


private:
	UPROPERTY(SaveGame)
	TArray<FQuestProgressData> QuestProgressItems;

	// 퀘스트 ID -> 인덱스 매핑 캐시
	UPROPERTY(Transient)
	mutable TMap<FGameplayTag, int32> QuestIndexMap;

	UPROPERTY(Transient)
	mutable bool bCacheDirtyFlag = true;

public:
	// **** FFastArraySerializer 인터페이스 재정의 ****

	// 클라이언트에서 복제 완료 후 호출되는 함수
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
	{
		bCacheDirtyFlag = true;
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FQuestProgressData, FQuestFastArray>(QuestProgressItems, DeltaParms, *this);
	}


	// **** 서버 전용 ****


	// 서버 전용 함수입니다. 권위 있는 쪽에서만 호출되어야 합니다.
	void AddItem(const FQuestProgressData& NewItem)
	{
		FQuestProgressData& Item = QuestProgressItems.Add_GetRef(NewItem);
		MarkItemDirty(Item);

		bCacheDirtyFlag = true;
	}

	// 서버 전용 함수입니다. 권위 있는 쪽에서만 호출되어야 합니다.
	bool RemoveItem(const FGameplayTag& InQuestID)
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


	// **** 공용 API 함수 ****


	// 게터
	FORCEINLINE const FQuestProgressData* Find(const FGameplayTag& QuestID) const
	{
		return Internal_Find(QuestID);
	}

	FORCEINLINE const TArray<FQuestProgressData>& GetItems() const
	{
		return QuestProgressItems;
	}

	FORCEINLINE int32 Num() const 
	{
		return QuestProgressItems.Num();
	}

	FORCEINLINE bool IsEmpty() const
	{
		return QuestProgressItems.Num() == 0;
	}

	// 범위 기반 for 지원
	auto begin() { return QuestProgressItems.begin(); }
	auto end() { return QuestProgressItems.end(); }

	auto begin() const { return QuestProgressItems.begin(); }
	auto end() const { return QuestProgressItems.end(); }

	 
	// 진행도를 업데이트 하는 함수 (서버 전용)
	bool UpdateProgressData(FQuestProgressData& InData)
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
	
	// 진행도 타입만 업데이트
	bool UpdateProgressData(const FGameplayTag& QuestID, EQuestProgress ProgressType)
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

	// 진행도 데이터만 업데이트
	bool UpdateProgressData(const FGameplayTag& QuestID, const FGameplayTag& ObjID, int32 NewValue)
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

	bool UpdateProgressData(const FGameplayTag& QuestID, const FGameplayTag& ObjID, EQuestProgress ProgressType, int32 NewValue)
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


	// **** 마이그레이션 함수 ****
	
	// **** 초기화 및 마이그레이션 통합 함수 ****
	void InitializeFromSaveData(const TSet<FGameplayTag>& ValidQuestTags)
	{
		bool bAnyChanged = false;

		for (int32 i = QuestProgressItems.Num() - 1; i >= 0; --i)
		{
			if (!ValidQuestTags.Contains(QuestProgressItems[i].GetQuestID()))
			{
				QuestProgressItems.RemoveAt(i);
				bAnyChanged = true;
				continue;
			}

			if (QuestProgressItems[i].MigrateToLatest())
			{
				MarkItemDirty(QuestProgressItems[i]); // 개별 아이템 변경 알림
				bAnyChanged = true;
			}
		}

		RebuildCache();

		if (bAnyChanged)
		{
			MarkArrayDirty();
		}
	}


private:
	/**
	 * @brief QuestID에 해당되는 FQuestProgressData의 값을 질의하고 받습니다
	 *
	 * @note 내부적으로 선형 검색과 맵 검색을 혼합하여 최적의 성능을 도모합니다.
	 *
	 * @param InQuestID 찾고자 하는 퀘스트의 GameplayTag ID
	 * @return FQuestProgressData* 해당 퀘스트의 진행 데이터 포인터 (없으면 nullptr)
	 */
	const FQuestProgressData* Internal_Find(const FGameplayTag& InQuestID) const
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

	// Effective C++ 패턴을 적용한 const_cast 패턴
	// 외부 공용 함수에 const를 적용해 더 안전하고 생산성 있는 구현이 가능
	FQuestProgressData* Internal_Find(const FGameplayTag& InQuestID)
	{
		if (!InQuestID.IsValid()) return nullptr;

		const FQuestProgressData* Result = const_cast<const FQuestFastArray*>(this)->Internal_Find(InQuestID);
		return const_cast<FQuestProgressData*>(Result);
	}



	// 캐시 재구축 함수
	void RebuildCache() const
	{
		QuestIndexMap.Empty(QuestProgressItems.Num());
		for (int32 i = 0; i < QuestProgressItems.Num(); ++i)
		{
			QuestIndexMap.Add(QuestProgressItems[i].GetQuestID(), i);
		}

		bCacheDirtyFlag = false;
	}
};

// FFastArraySerializer 특수화
template<>
struct TStructOpsTypeTraits<FQuestFastArray> : public TStructOpsTypeTraitsBase2<FQuestFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


/*
* @brief UMG(UI)와 통신하기 위한 데이터 구조체 (DTO)
* @note BlueprintType으로 UMG에서 쉽게 읽을 수 있게 합니다.
* 
* 이 구조체는 의도적으로 128 바이트에 근접하게 크기를 유지했습니다.
*/
USTRUCT(BlueprintType)
struct FQuestLogEntry
{
    GENERATED_BODY()

public:
    // 퀘스트 고유 ID
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FGameplayTag QuestID;

	// 퀘스트 유형
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuestType QuestType; // Main, Side, Daily...

	// 퀘스트 우선순위
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	uint8 Priority = 0;

    // 퀘스트 상태
    // UI가 이 상태를 보고 "In-Progress", "Complete", "Turn-In" 배지를 표시합니다.
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuestProgress CurrentState = EQuestProgress::None;

    // 퀘스트 제목
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText Title;

    // 퀘스트 설명
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText Description;

	// 퀘스트 아이콘
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TSoftObjectPtr<UTexture2D> Icon;

    // Manager가 ObjectiveConfig와 ProgressData를 조합해 만들어줍니다.
	// 특정 상태에서만 업데이트됩니다.
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FText> FormattedObjectives;
};

/**
 * @brief GMS를 통해 전송될 퀘스트 관련 '표준 이벤트 메시지'입니다.
 * "택배 상자" 역할을 하며, 이벤트에 대한 핵심 정보를 담습니다.
 */
USTRUCT(BlueprintType)
struct FQuestMessage_Generic
{
	GENERATED_BODY()

	/**
	 * @brief 이 이벤트를 발생시킨 주체입니다.
	 * 예: 퀘스트를 진행 중인 플레이어, 퀘스트 AI 등
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Message")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	/**
	 * @brief 이 이벤트의 대상이 된 액터입니다.
	 * 예: 방금 죽은 AI, 방금 수집한 아이템 액터, 방금 진입한 구역(트리거)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Message")
	TObjectPtr<AActor> TargetActor = nullptr;

	/**
	 * @brief 대상(TargetActor)의 핵심 태그 컨테이너입니다.
	 * QuestObjective가 이 태그를 자신의 Config와 비교합니다.
	 * 예: "AI.Enemy.Zombie", "Item.Quest.Letter", "Zone.Dormitory.301"
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Message")
	FGameplayTagContainer TargetTags;

	/**
	 * @brief 이벤트와 관련된 수량입니다.
	 * 예: 아이템 5개 수집, 100 데미지 등
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Message")
	int32 Amount = 0;
	
	/**
	 * @brief 함께 보낼 텍스트입니다. (필요 시만 사용)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Message")
	FText text;
};

/**
 * @brief 퀘스트 시스템 알림 전용 메시지 구조체
 * UI는 이 메시지를 받아서 토스트(Toast) 등을 띄우기만 하면 됩니다.
 */
USTRUCT(BlueprintType)
struct FQuestNotificationMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quest|Notification")
	FGameplayTag QuestID;

	// 화면에 출력될 최종 텍스트 (예: "쥐 잡기 목표 완료 (1/3)")
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Notification")
	FText NotificationText;
};