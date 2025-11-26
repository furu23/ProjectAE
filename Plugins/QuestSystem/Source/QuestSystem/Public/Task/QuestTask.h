// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestTask.generated.h"

/**
 * 퀘스트가 월드에 요청할 작업의 추상 기반 클래스입니다.
 * UQuestObjectiveConfig 내부에서 'Instanced'로 인라인 생성됩니다.
*/

UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestTask : public UObject
{
	GENERATED_BODY()

public:
	// 이 태스크가 실제로 실행될 때 호출될 함수입니다.
	// WorldDirector가 이 함수를 호출해줍니다.
	virtual void Execute(UObject* WorldContextObject);
};
 
/*

	[예시]

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
	virtual void Execute(UQuestWorldDirector* WorldDirector) override
	{
		// WorldDirector에게 "이 아이템을 이 태그 위치에 스폰해줘"라고 요청
		// WorldDirector->SpawnActorAtTaggedLocation(ItemToSpawn, SpawnLocationTag);
		UE_LOG(LogTemp, Log, TEXT("Executing Task: Spawn %s at %s"), *ItemToSpawn->GetName(), *SpawnLocationTag.ToString());
	}
};


*/