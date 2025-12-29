// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "ObjectiveConfig_Kill.generated.h"

/**
 * @brief 킬 오브젝트의 기본 데이터 에셋 제공형입니다.
 */
UCLASS()
class UObjectiveConfig_Kill : public UQuestObjectiveConfig
{
	GENERATED_BODY()

public:
	// 클래스 기본값을 위한 생성자
	UObjectiveConfig_Kill();

	UPROPERTY(EditInstanceOnly, Category = "Objective|Kill", meta = (ToolTip = "처치 목표 태그입니다."))
	FGameplayTag ListenTag;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Kill", meta = (ToolTip = "처치 대상의 태그입니다."))
	FGameplayTagContainer TargetTags;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Kill", meta = (MultiLine = true, ToolTip = "목표의 포맷 텍스트입니다."))
    FText DescriptionFormat;

    // 완료되었을 때 보여줄 텍스트 (옵션)
    UPROPERTY(EditInstanceOnly, Category = "Objective|Kill", meta = (ToolTip = "목표 완료 시 보일 완료 메세지입니다."))
    FText CompletedDescription;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Kill", meta = (ToolTip = "목표 처치 수 입니다."))
	int32 RequiredNumber;

	// ... 조건이 있는 처치, 특정 무기로 처치 등은 이 클래스를 상속해서 추가 구현해주세요.

	// 포맷 전달 함수를 오버라이드합니다.
	virtual FText GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const override;
};
