// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "QuestObjectiveConfig.generated.h"

class UQuestObjective;
class UQuestTask;

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

	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToolTip = "이 객체가 반환할 런타임 객체 클래스입니다. (기본값 = 일반적으로 이미 정의된 안전한 클래스)"))
	TSubclassOf<UQuestObjective> ObjectiveClass;

	// 이 퀘스트가 사용할 런타임 객체, QuestObjective를 반환합니다. ex) QuestObjective_Kill::StaticClass();
	TSubclassOf<UQuestObjective> GetQuestObjectiveClass() const;

	// 전달용 오브젝티브 포맷화된 문자열을 반환합니다.
	virtual FText GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const;
};