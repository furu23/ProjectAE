#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
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


	// **** 버전 관리용 프로퍼티 ****

	UPROPERTY(SaveGame)
    int32 Version = QUEST_DATA_CURRENT_VERSION;


	// **** 실제 퀘스트 진행 상태 프로퍼티 ****

	// 퀘스트 고유 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Quest")
	FGameplayTag QuestID = FGameplayTag::EmptyTag;
	
	// 현재 진행도의 단계
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Quest")
	EQuestProgress ProgressType = EQuestProgress::None;

	// 진행도의 실제 진행상황
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Quest")
	TArray<FQuestObjectiveData> Objectives;


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
		return false;
	}


	// **** 편의 함수 모음 ****

	// ObjectiveID에 해당되는 FQuestObjectiveData의 값을 질의하고 받습니다
	FQuestObjectiveData* FindObjective(FGameplayTag ObjID)
	{
		return Objectives.FindByKey(ObjID);
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
	static constexpr int32 LinearSearchThreshold = 20;


private:
	UPROPERTY(SaveGame)
	TArray<FQuestProgressData> QuestProgressItems;

	// 퀘스트 ID -> 인덱스 매핑 캐시
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> QuestIndexMap;


public:
	// **** FFastArraySerializer 인터페이스 재정의 ****

	// 클라이언트에서 복제 완료 후 호출되는 함수
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
	{
		RebuildCache();
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

		// 서버 캐시 갱신
		QuestIndexMap.Add(NewItem.QuestID, QuestProgressItems.Num() - 1);
	}

	// 서버 전용 함수입니다. 권위 있는 쪽에서만 호출되어야 합니다.
	bool RemoveItem(FGameplayTag InQuestID)
	{
		// 캐시를 통해 빠르게 찾음
		if (const int32* IdxPtr = QuestIndexMap.Find(InQuestID))
		{
			int32 Idx = *IdxPtr;
			if (QuestProgressItems.IsValidIndex(Idx) && QuestProgressItems[Idx].QuestID == InQuestID)
			{
				MarkItemDirty(QuestProgressItems[Idx]);
				QuestProgressItems.RemoveAt(Idx);
				MarkArrayDirty();

				RebuildCache();
				return true;
			}
		}
		return false;
	}


	// **** 공용 쿼리 함수 ****

	/**
	 * @brief QuestID에 해당되는 FQuestProgressData의 값을 질의하고 받습니다
	 * 
	 * @note 내부적으로 선형 검색과 맵 검색을 혼합하여 최적의 성능을 도모합니다.
	 * 
	 * @param InQuestID 찾고자 하는 퀘스트의 GameplayTag ID
	 * @return FQuestProgressData* 해당 퀘스트의 진행 데이터 포인터 (없으면 nullptr)
	 */
	const FQuestProgressData* Find(FGameplayTag InQuestID)
	{
		if (!InQuestID.IsValid()) return nullptr;

		if (QuestProgressItems.Num() <= LinearSearchThreshold)
		{
			return QuestProgressItems.FindByKey(InQuestID);
		}


		if (const int32* IdxPtr = QuestIndexMap.Find(InQuestID))
		{
			if (QuestProgressItems.IsValidIndex(*IdxPtr) && QuestProgressItems[*IdxPtr].QuestID == InQuestID)
			{
				return &QuestProgressItems[*IdxPtr];
			}
		}

		RebuildCache();
		if (const int32* IdxPtr = QuestIndexMap.Find(InQuestID))
		{
			if (QuestProgressItems.IsValidIndex(*IdxPtr))
			{
				if (QuestProgressItems[*IdxPtr].QuestID == InQuestID)
				{
					return &QuestProgressItems[*IdxPtr];
				}
			}
		}
		return nullptr;
	}

	bool Contains(FGameplayTag InQuestID)
	{
		return Find(InQuestID) != nullptr;
	}

	bool IsEmpty() const
	{
		return QuestProgressItems.Num() == 0;
	}

	bool Update


	// **** 마이그레이션 함수 ****
	
	// **** 초기화 및 마이그레이션 통합 함수 ****
	void InitializeFromSaveData(const TSet<FGameplayTag>& ValidQuestTags)
	{
		bool bAnyChanged = false;

		for (int32 i = QuestProgressItems.Num() - 1; i >= 0; --i)
		{
			if (!ValidQuestTags.Contains(QuestProgressItems[i].QuestID))
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


	// **** 접근자 함수 ****
	const TArray<FQuestProgressData>& GetItems() const { return QuestProgressItems; }

private:
	// 캐시 재구축 함수
	void RebuildCache() const
	{
		QuestIndexMap.Empty(QuestProgressItems.Num());
		for (int32 i = 0; i < QuestProgressItems.Num(); ++i)
		{
			QuestIndexMap.Add(QuestProgressItems[i].QuestID, i);
		}
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