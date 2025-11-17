// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Objectives/QuestObjectiveConfig.h"
#include "ObjectiveConfig_Interact.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API UObjectiveConfig_Interact : public UQuestObjectiveConfig
{
	GENERATED_BODY()

public:
	// 클래스 기본값을 위한 생성자
	UObjectiveConfig_Interact();

	UPROPERTY(EditInstanceOnly, Category = "Objective|Interact", meta = (ToolTip = "상호작용 태그입니다."))
	FGameplayTag ListenTag;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Interact", meta = (ToolTip = "상호작용 대상의 태그입니다."))
	FGameplayTagContainer TargetTags;
};
