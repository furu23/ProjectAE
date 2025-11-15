// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestObjectiveConfig.generated.h"

class UAEQuestObjective;

/**
 * @brief 퀘스트 목표의 설계도를 정의하는 데이터 에셋 추상 클래스입니다.
 * @note 이 클래스를 상속받아 구체적인 퀘스트 목표 설계도 클래스를 만들어야 합니다.
 */
UCLASS(Abstract)
class PROJECTAE_API UQuestObjectiveConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToopTip = "FQuestProgressData 의 TMap 에 Key 값이 됩니다."))
	FGameplayTag ObjectiveID;

	UPROPERTY(EditInstanceOnly, Category = "Objective", meta = (ToolTip = "True 로 설정하면 이 퀘스트 목표가 활성화될 때 월드 시스템에 태스크를 보내도록 트리거합니다."))
	bool bNotifyWorldSystemOnActivation = false;

	// UPROPERTY(EditInstanceOnly, Category = "Objective",
		// meta = (EditCondition = "bNotifyWorldSystemOnActivation", ToolTip = "이 퀘스트가 활성화될 때 월드 시스템에 보낼 태스크들의 배열입니다."))
	// TArray<TObjectPtr<UQuestWorldTask>> TaskOnActivation() const;

	// 이 퀘스트가 사용할 런타임 객체, QuestObjective를 반환합니다. ex) QuestObjective_Kill::StaticClass();
	virtual TSubclassOf<UAEQuestObjective> GetQuestObjectiveClass() const;
};
