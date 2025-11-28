// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "QuestObject.generated.h"

class UDA_QuestBase;
class UQuestManagerSubSystem;
class UQuestObjective;
class UQuestTask;

DECLARE_DELEGATE_OneParam(FOnQuestObjectChangedSignature, const FGameplayTag&);
DECLARE_DELEGATE_OneParam(FOnRequestWorldTasksSignature, const TArray<TObjectPtr<UQuestTask>>&);

/**
 * @brief 퀘스트 진행에 대한 런타임 객체입니다. InProgresss 상태의 객체들만 이 객체를 가지며, 실제 퀘스트 목표를 관리하는 역할을 맡습니다.
 */
UCLASS()
class QUESTSYSTEM_API UQuestObject : public UObject
{
	GENERATED_BODY()

public:
	// **** 델리게이트 ****
	
	// 퀘스트 상태 변경을 알리기 위한 델리게이트
	FOnQuestObjectChangedSignature OnQuestObjectChangedDelegate;

	FOnRequestWorldTasksSignature OnRequestWorldTasksDelegate;


	// **** 상위 객체 호출 함수 ****

	// 퀘스트 런타임 객체를 초기화합니다.
	virtual void Initialize(UDA_QuestBase* DefRef, UQuestManagerSubSystem* Manager);

	// Objective 배열을 순회하며 활성화 시킵니다. 델리게이트에 OnObjectCompleted 함수를 바인드합니다.
	virtual void Activate(UObject* WorldContext);

	// 모든 배열을 비활성화시킵니다.
	virtual void DeActivate();

	// 퀘스트가 완료되었는지 확인합니다.
	virtual bool CheckQuestCompletion();

protected:
	// **** 초기화될 기본 프로퍼티 ****

	// 이 퀘스트 런타임 객체의 설계도가 되는 UDA_QuestBase 객체에 대한 참조입니다.
	TObjectPtr<UDA_QuestBase> Definition;

	// 퀘스트의 하위 목표 객체 배열입니다.
	TArray<TObjectPtr<UQuestObjective>> Objectives;

	// 퀘스트 서브시스템에 대한 캐싱입니다. 유효성 검사 필요.
	TObjectPtr<UQuestManagerSubSystem> CachedQuestSys;


	// **** 주요 기능 함수들 ****

	// 하위 객체에서 퀘스트 완료 시 호출됩니다. 델리게이트를 통해 호출됩니다.
	virtual void OnObjectiveCompleted(UQuestObjective* Objective);


	// **** 델리게이트 바인딩 함수 ****

	void OnObjectiveRequestingTasks(const TArray<TObjectPtr<UQuestTask>>& TasksToExecute);
};