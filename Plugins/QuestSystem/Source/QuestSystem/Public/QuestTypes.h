#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

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

USTRUCT(BlueprintType)
struct FQuestProgressData
{
	GENERATED_BODY()
	
	// 현재 진행도의 단계
	EQuestProgress ProgressType;

	// 진행도의 실제 진행상황
	TMap<FGameplayTag, int32> ObjectProgress; 
};

/*
* @brief UMG(UI)와 통신하기 위한 데이터 구조체 (DTO)
* @note BlueprintType으로 UMG에서 쉽게 읽을 수 있게 합니다.
*/
USTRUCT(BlueprintType)
struct FQuestLogEntry
{
    GENERATED_BODY()

    // 1. 퀘스트 고유 ID (UI가 '보상 받기' 등을 클릭했을 때 Manager에게 돌려줄 ID)
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FGameplayTag QuestID;

    // 2. 퀘스트 제목 (UDA_QuestBase에서 가져옴)
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText Title;

    // 3. 퀘스트 설명 (UDA_QuestBase에서 가져옴)
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText Description;

    // 4. 퀘스트 상태 (FQuestProgressData에서 가져옴)
    // UI가 이 상태를 보고 "In-Progress", "Complete", "Turn-In" 배지를 표시합니다.
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuestProgress CurrentState = EQuestProgress::None;

    // Manager가 ObjectiveConfig와 ProgressData를 조합해 만들어줍니다.
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