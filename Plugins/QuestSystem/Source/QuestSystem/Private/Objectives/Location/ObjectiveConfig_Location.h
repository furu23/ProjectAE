// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "ObjectiveConfig_Location.generated.h"

/**
 * 
 */
UCLASS()
class UObjectiveConfig_Location : public UQuestObjectiveConfig
{
	GENERATED_BODY()
	
public:
	// 클래스 기본값을 위한 생성자
	UObjectiveConfig_Location();

	UPROPERTY(EditInstanceOnly, Category = "Objective|Location", meta = (ToolTip = "채널 태그입니다."))
	FGameplayTag ListenTag;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Location", meta = (ToolTip = "비교할 목표 태그입니다."))
	FGameplayTagContainer TargetTags;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Location", meta = (MultiLine = true, ToolTip = "목표의 포맷 텍스트입니다."))
    FText DescriptionFormat;

    // 완료되었을 때 보여줄 텍스트 (옵션)
    UPROPERTY(EditInstanceOnly, Category = "Objective|Location", meta = (ToolTip = "목표 완료 시 보일 완료 메세지입니다."))
    FText CompletedDescription;

	// 포맷 전달 함수를 오버라이드합니다.
	virtual FText GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const override;
};
