// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestTypes.h"
#include "QuestObjective.generated.h"

class UQuestObjectiveConfig;
class UQuestManagerSubSystem;
class UQuestObjective;
class UQuestTask;

DECLARE_DELEGATE_OneParam(FOnObjectiveCompletedSignature, UQuestObjective* /* CompletedObjective */);
DECLARE_DELEGATE_OneParam(FOnRequestWorldTasksSignature, const TArray<TObjectPtr<UQuestTask>>& /* TasksToExecute */);

/**
 * @brief 런타임, In-Raid 레벨에서 실제 게임플레이 퀘스트 진행 완료에 관련된 클래스입니다.
 * 
 * 이 클래스는 직접적으로 GameplayMessageSubSystem을 통해 인-레이드 레벨 게임플레이와 통신해 목표 값을 갱신합니다.
 * 
 * @note 목표 값을 저장해야 하는 목표라면, 로드 시 구체적 클래스에서 반드시 CachedQuestSys에 쿼리해 진행도를 동기화해야 합니다.
 * @note 구체 클래스에서 Activate 함수에서 bHasFiredCompletion을 체크하여 리턴할 것을 권장합니다.
 */
UCLASS(Abstract)
class QUESTSYSTEM_API UQuestObjective : public UObject
{
	GENERATED_BODY()
public:
	// **** 목표 완료 시 방송용 델리게이트 ****

	// 목표 완료 시 델리게이트
	FOnObjectiveCompletedSignature OnObjectiveCompleteDelegate;

	// 태스크 버블링 용 델리게이트
	FOnRequestWorldTasksSignature OnRequestTaskSignatureDelegate;


	// **** 외부 호출 함수 (퀘스트 모듈 내부에서만 사용할 것을 권장합니다.)****

	// 초기화 함수
	virtual void Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectQuestID);

	// QuestObject 에서 호출할 GMS 구독 함수
	virtual void Activate(UObject* WorldContext);
	
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


	// **** 초기화 및 내부 멤버 ****

	// 현재 퀘스트의 퀘스트 ID를 저장
	FGameplayTag QuestID;

	// 설정 데이터 참조
	const UQuestObjectiveConfig* ObjectiveConfig;

	// 진행도 포인터 반환을 위한 퀘스트 시스템 캐시
	TObjectPtr<UQuestManagerSubSystem> CachedQuestSys;

	// 이미 완료 함수가 호출되었는 지를 확인하는 bool 프로퍼티
	bool bHasFiredCompletion = false;
};
