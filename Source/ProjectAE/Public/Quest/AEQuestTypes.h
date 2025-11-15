#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "AEQuestTypes.generated.h"

UENUM(BlueprintType)
enum class EQuestProgress : uint8
{
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

// UMG(UI)와 통신하기 위한 데이터 구조체 (DTO)
// BlueprintType으로 UMG에서 쉽게 읽을 수 있게 합니다.
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
    EQuestProgress CurrentState;

    // 5. (가장 중요) 가공된 목표 텍스트
    // 예: "좀비 처치 (7 / 10)"
    // Manager가 ObjectiveConfig와 ProgressData를 조합해 만들어줍니다.
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FText> FormattedObjectives;
    
    // 6. 보상 정보 (UDA_QuestBase에서 가져옴)
    // UI가 보상 아이콘 등을 표시할 수 있게 합니다.
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuestProgress RewardData;
};