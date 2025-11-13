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
	
public:
	// 현재 진행도의 단계
	EQuestProgress ProgressType;

	// 진행도의 실제 진행상황
	TMap<FGameplayTag, int32> ObjectProgress; 
};

UCLASS(Abstract)
class PROJECTAE_API UAEQuestTypes : public UObject
{
	GENERATED_BODY()

public:
};