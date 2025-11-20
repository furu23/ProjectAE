// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "QuestObjectiveConfig.generated.h"

class UQuestObjective;

/**
 * @brief 퀘스트 목표의 설계도를 정의하는 데이터 에셋 추상 클래스입니다.
 * @note 이 클래스를 상속받아 구체적인 퀘스트 목표 설계도 클래스를 만들어야 합니다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestObjectiveConfig : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToopTip = "FQuestProgressData 의 TMap 에 Key 값이 됩니다."))
	FGameplayTag ObjectiveID;

	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToolTip = "True 로 설정하면 이 퀘스트 목표가 활성화될 때 월드 시스템에 태스크를 보내도록 트리거합니다."))
	bool bNotifyWorldSystemOnActivation = false;

	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToolTip = "이 객체가 반환할 런타임 객체 클래스입니다. (기본값 = 일반적으로 이미 정의된 안전한 클래스)"))
	TSubclassOf<UQuestObjective> ObjectiveClass;

	// UPROPERTY(EditInstanceOnly, Instanced, Category = "Objective",
		// meta = (EditCondition = "bNotifyWorldSystemOnActivation", ToolTip = "이 퀘스트가 활성화될 때 월드 시스템에 보낼 태스크들의 배열입니다."))
	// TArray<TObjectPtr<UQuestWorldTask>> TaskOnActivation() const;

	// 이 퀘스트가 사용할 런타임 객체, QuestObjective를 반환합니다. ex) QuestObjective_Kill::StaticClass();
	TSubclassOf<UQuestObjective> GetQuestObjectiveClass() const;

	// 전달용 오브젝티브 포맷화된 문자열을 반환합니다.
	virtual FText GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const;
};


/*
// QuestWorldTask.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestWorldTask.generated.h"

class UQuestWorldDirector; // "월드 시스템"의 전방 선언

/**
 * 퀘스트가 월드에 요청할 작업(명령)의 추상 기반 클래스입니다.
 * UQuestObjectiveConfig 내부에서 'Instanced'로 인라인 생성됩니다.
*/
/*
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class YOURPROJECT_API UQuestWorldTask : public UObject
{
	GENERATED_BODY()

public:
	// 이 태스크가 실제로 실행될 때 호출될 함수입니다.
	// WorldDirector가 이 함수를 호출해줍니다.
	virtual void Execute(UQuestWorldDirector* WorldDirector)
	{
		// 자식 클래스에서 이 함수를 재정의(override)해야 합니다.
	}
};

 
// Task_SpawnQuestItem.h
#pragma once

#include "CoreMinimal.h"
#include "QuestWorldTask.h"
#include "GameplayTagContainer.h"
#include "Task_SpawnQuestItem.generated.h"

UCLASS() // EditInlineNew가 부모에 있으므로 자식은 UCLASS()만 있어도 됩니다.
class YOURPROJECT_API UTask_SpawnQuestItem : public UQuestWorldTask
{
	GENERATED_BODY()

protected:
	// 디자이너가 퀘스트 Config에서 인라인으로 채워넣을 데이터
	UPROPERTY(EditAnywhere, Category="Task")
	TSubclassOf<AActor> ItemToSpawn; // 스폰할 아이템 액터

	UPROPERTY(EditAnywhere, Category="Task")
	FGameplayTag SpawnLocationTag; // "Dormitory_Room_301" 같은 스폰 위치 태그

public:
	// [핵심] 실제 로직
	virtual void Execute(UQuestWorldDirector* WorldDirector) override
	{
		// WorldDirector에게 "이 아이템을 이 태그 위치에 스폰해줘"라고 요청
		// WorldDirector->SpawnActorAtTaggedLocation(ItemToSpawn, SpawnLocationTag);
		UE_LOG(LogTemp, Log, TEXT("Executing Task: Spawn %s at %s"), *ItemToSpawn->GetName(), *SpawnLocationTag.ToString());
	}
};
*/