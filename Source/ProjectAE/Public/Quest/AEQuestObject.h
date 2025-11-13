// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AEQuestTypes.h"
#include "AEQuestObject.generated.h"

class UDA_QuestBase;
class UQuestManagerSubSystem;
class UAEQuestObjective;

/**
 * @brief 퀘스트 진행에 대한 런타임 객체입니다. InProgresss 상태의 객체들만 이 객체를 가지며, 실제 퀘스트 목표를 관리하는 역할을 맡습니다.
 */
UCLASS()
class PROJECTAE_API UAEQuestObject : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(UDA_QuestBase* DefRef, FQuestProgressData* ProgressRef, UQuestManagerSubSystem* Manager);

protected:
	// **** 초기화될 기본 프로퍼티 ****

	// 이 퀘스트 런타임 객체의 설계도가 되는 UDA_QuestBase 객체에 대한 참조입니다.
	TObjectPtr<UDA_QuestBase> Definition;

	// 퀘스트 진행도에 대한 레퍼런스를 가집니다.
	FQuestProgressData* ProgressDataRef;

	// 퀘스트의 하위 목표 객체 배열입니다.
	TArray<TObjectPtr<UAEQuestObjective>> Objectives;

	// 퀘스트 서브시스템에 대한 캐싱입니다. 유효성 검사 필요.
	TObjectPtr<UQuestManagerSubSystem> CachedQuestSys;


	// **** 퀘스트 진행에 따른 호출 함수 관련 ****

	// Objective 배열을 순회하며 활성화 시킵니다. 델리게이트에 OnObjectCompleted 함수를 바인드합니다.
	virtual void Activate(UObject* WorldContext);

	// 모든 배열을 비활성화시킵니다.
	virtual void DeActivate();

	// 하위 객체에서 퀘스트 완료 시 호출됩니다. 델리게이트를 통해 호출됩니다.
	virtual void OnObjectCompleted(UAEQuestObjective* Objective);

	// 퀘스트가 완료되었는지 확인합니다.
	virtual void CheckQuestCompletion();

private:
	// **** 내부 진행도 추적 관련 ****

	// 총 Objective의 수를 측정해둡니다.
	int32 TotalObjectiveNumber;

	// 현재 완료된 Objective의 수를 측정합니다.
	int32 CurrentObjectiveNumber = 0;
};
