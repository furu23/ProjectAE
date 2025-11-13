// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AEQuestTypes.h"
#include "AEQuestObjective.generated.h"

class UQuestObjectiveConfig;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, UAEQuestObjective);

/**
 * @brief 런타임, In-Raid 레벨에서 실제 게임플레이 퀘스트 진행 완료에 관련된 클래스입니다.
 * 
 * @note 이 클래스는 직접적으로 GameplayMessageSubSystem을 통해 인-레이드 레벨 게임플레이와 통신해 목표 값을 갱신합니다.
 */
UCLASS()
class PROJECTAE_API UAEQuestObjective : public UObject
{
	GENERATED_BODY()
public:
	// **** 외부 호출 함수 (퀘스트 시스템 내부에서만 사용해야 합니다.)****

	// 초기화 함수
	virtual void Initialize();

	// QuestObject 에서 호출할 GMS 구독 함수
	virtual void Activate(UObject* WorldContext);
	
	// QuestObject 에서 호출할 비활성화 함수
	virtual void DeAcitvate();

	
	// **** 목표 완료 시 방송용 델리게이트 ****

	FOnObjectiveCompleted OnObjectiveComplete;

protected:

	virtual bool IsComplete() const;

	// GMS 구독 콜백 함수
	//virtual void OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message)

	// GMS 구독 관리 핸들
	FGameplayMessageListenerHandle GMSListenHandle;

	// 설정 데이터 참조
	TObjectPtr<UQuestObjectiveConfig> ObjectiveConfig;

	// 진행도 포인터
	FQuestProgressData* ProgressDataRef;
};
