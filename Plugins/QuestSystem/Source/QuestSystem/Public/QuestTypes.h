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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "퀘스트 설명입니다."))
    FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "퀘스트 아이콘입니다."))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
	FGameplayTagContainer PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="System")
    EQuestType QuestType; // Main, Side, Daily...

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
	uint8 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Policy")
	bool bIsStartingQuest = false;

#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dev")
    FText DevComment;
#endif
};


USTRUCT(BlueprintType)
struct FQuestObjectiveData
{
    GENERATED_BODY()

public:
	FQuestObjectiveData() {}

	explicit FQuestObjectiveData(const FGameplayTag& ObjectiveID) : ObjectiveID(ObjectiveID) {}
	explicit FQuestObjectiveData(const FGameplayTag& ObjectiveID, int32 Count) : ObjectiveID(ObjectiveID), Count(Count) {}


    UPROPERTY(SaveGame)
    FGameplayTag ObjectiveID;

    UPROPERTY(SaveGame)
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

	// 진행도의 실제 진행상황
	UPROPERTY(SaveGame)
	TArray<FQuestObjectiveData> Objectives;
	
	// 현재 진행도의 단계
	UPROPERTY(SaveGame)
	EQuestProgress ProgressType = EQuestProgress::None;




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
		if (Version < 1)
		{
			// 버전 1 마이그레이션 로직 (예시)
			// bCacheDirtyFlag = true; // 예시: 캐시 재구축이 필요함을 표시
			// 현재는 변경 사항이 없으므로 아무 작업도 하지 않습니다.
		}

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
struct FQuestContext
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Context")
    TObjectPtr<AActor> Instigator = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    TObjectPtr<UObject> TargetObject = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    FGameplayTagContainer TargetTags;

    // [옵션] 사건의 수치 (데미지 양, 획득 개수 등)
    UPROPERTY(BlueprintReadWrite, Category = "Context")
    int32 Amount = 1;

    // [옵션] 추가 정보 (무엇으로 죽였나? 장소는 어디인가?)
    UPROPERTY(BlueprintReadWrite, Category = "Context")
    FGameplayTagContainer ContextTags;

    // 생성자 (편의용)
    FQuestContext() {}
    FQuestContext(AActor* InInstigator, UObject* InTarget) 
        : Instigator(InInstigator), TargetObject(InTarget) {}
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
 * @brief 퀘스트 액션 실행 시 전달되는 컨텍스트 정보 구조체
 * @note 네트워크 직렬화가 가능하도록 설계되었습니다.
 */
USTRUCT(BlueprintType)
struct FQuestActionContext
{
    GENERATED_BODY()

public:
    FQuestActionContext()
        : Instigator(nullptr), TargetActor(nullptr), Amount(0), Location(FVector::ZeroVector), PolicyFlags(0) {}

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    TWeakObjectPtr<AActor> Instigator; 

    UPROPERTY(BlueprintReadWrite, Category = "Context")
	TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(BlueprintReadWrite, Category = "Context")
	TWeakObjectPtr<UObject> SourceObject;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
    FGameplayTagContainer TargetTag;

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    FGameplayTag EventTag;

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    int32 Amount;

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    FVector_NetQuantize10 Location; 

    UPROPERTY(BlueprintReadWrite, Category = "Context")
    uint8 PolicyFlags;

	AActor* GetInstigator() const { return Instigator.Get(); }
	AActor* GetTargetActor() const { return TargetActor.Get(); }
    
    bool HasPolicy(uint8 FlagToCheck) const { return (PolicyFlags & FlagToCheck) != 0; }
    void AddPolicy(uint8 FlagToAdd) { PolicyFlags |= FlagToAdd; }
	void RemovePolicy(uint8 FlagToRemove) { PolicyFlags &= ~FlagToRemove; }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

/**
 * 템플릿 특수화, 언리얼 엔진에게 이 구조체가 커스텀 NetSerialize를 사용한다고 알림
 */
template<>
struct TStructOpsTypeTraits<FQuestActionContext> : public TStructOpsTypeTraitsBase2<FQuestActionContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
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