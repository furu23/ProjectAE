// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestTypes.h"
#include "QuestObjective.generated.h"

class UQuestObjectiveConfig;
// class UQuestWorldTask

DECLARE_DELEGATE_OneParam(FOnObjectiveCompleted, UQuestObjective* /* CompletedObjective */);
// DECLARE_DELEGATE_OneParam(FOnRequestWorldTasksSignature, const TArray<TObjectPtr<UQuestWorldTask>> /* TasksToExecute */);

/**
 * @brief 런타임, In-Raid 레벨에서 실제 게임플레이 퀘스트 진행 완료에 관련된 클래스입니다.
 * @note 이 클래스는 직접적으로 GameplayMessageSubSystem을 통해 인-레이드 레벨 게임플레이와 통신해 목표 값을 갱신합니다.
 */
UCLASS(Abstract)
class QUESTSYSTEM_API UQuestObjective : public UObject
{
	GENERATED_BODY()
public:
	// **** 목표 완료 시 방송용 델리게이트 ****

	// 목표 완료 시 델리게이트
	FOnObjectiveCompleted OnObjectiveCompleteDelegate;

	// 태스크 버블링 용 델리게이트
	// FOnRequestWorldTasksSignature OnRequestTaskSignatureDelegate;


	// **** 외부 호출 함수 (퀘스트 모듈 내부에서만 사용할 것을 권장합니다.)****

	// 초기화 함수
	virtual void Initialize(const UQuestObjectiveConfig* Config, FQuestProgressData* ProgressRef);

	// QuestObject 에서 호출할 GMS 구독 함수
	virtual void Activate(UObject* WorldContext) PURE_VIRTUAL(UQuestObjective::Activate)
	
	// QuestObject 에서 호출할 비활성화 함수
	virtual void DeActivate() PURE_VIRTUAL(UQuestObjective::DeActivate)

	// 완료 여부를 반환
	virtual bool IsComplete() const PURE_VIRTUAL(UQuestObjective::IsComplete, return false;)

protected:
	// **** GMS 관련 ****

	// GMS 구독 콜백 함수
	virtual void OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message) PURE_VIRTUAL(UQuestObjective::OnMessageReceived)

	// GMS 구독 관리 핸들
	FGameplayMessageListenerHandle GMSListenHandle;


	// **** 초기화 관련 멤버 ****

	// 설정 데이터 참조
	const UQuestObjectiveConfig* ObjectiveConfig;

	// 진행도 포인터
	FQuestProgressData* ProgressDataRef;
};
