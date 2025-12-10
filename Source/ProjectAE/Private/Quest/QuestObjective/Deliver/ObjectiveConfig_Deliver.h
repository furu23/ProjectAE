// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "ObjectiveConfig_Deliver.generated.h"

/**
 * @brief 아이템 제출에 관련된 퀘스트 목표입니다.
 * 
 * @note 기본적으로 제출을 감시하는 목표가 되지만 추가로 퀘스트 내용을 정의할 수 있습니다.
 * @see UQuestObjective_Deliver::GetRequirementInfo();
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UObjectiveConfig_Deliver : public UQuestObjectiveConfig
{
	GENERATED_BODY()

public:
	// 클래스 기본값을 위한 생성자
	UObjectiveConfig_Deliver();

	UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver", meta = (ToolTip = "청취할 GMS 채널 태그입니다."))
	FGameplayTag ListenTag;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver", meta = (ToolTip = "제출 이벤트 태그입니다. 가장 간단하게 구현할 시 사용합니다."))
	FGameplayTag TargetEventTag;


	/* 세부 정보를 필요로 할 때 */

	UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver", meta = (ToolTip = "True 로 설정하면 이 퀘스트 목표가 활성화된 후, 아이템 요구사항을 조회할 수 있습니다."))
	bool bBroadcastRequirementInfo = false;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver|Advance",
		meta = (EditCondition = "bBroadcastRequirementInfo", ToolTip = "목표 아이템의 ID 입니다."))
	FName ItemID = NAME_None;

	UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver|Advance",
		meta = (EditCondition = "bBroadcastRequirementInfo", ToolTip = "목표 아이템의 수량입니다."))
	int32 ItemAmount = -1;


    UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver", meta = (MultiLine = true, ToolTip = "목표의 포맷 텍스트입니다."))
    FText DescriptionFormat;

    // 완료되었을 때 보여줄 텍스트 (옵션)
    UPROPERTY(EditInstanceOnly, Category = "Objective|Deliver", meta = (ToolTip = "목표 완료 시 보일 완료 메세지입니다."))
    FText CompletedDescription;

	// 포맷 전달 함수를 오버라이드합니다.
	virtual FText GetFormattedObjectiveText(const FQuestProgressData& ProgressData) const override;



	// **** 외부 조회 함수 ****

	/**
	 * @brief 외부 시스템이 이 목표의 요구사항을 조회합니다.
	 * @param OutObjectiveTag (출력) 해당 목표의 오브젝트 태그
	 * @param OutItemID    (출력) 요구하는 아이템 ID
	 * @param OutItemAmount (출력) 요구하는 아이템 수량
	 * @return true 정보 조회 성공 (bBroadcastRequirementInfo가 true이며 데이터가 유효함)
	 * @return false 정보 조회 실패 (공개 설정이 꺼져있거나 데이터가 없음)
	 */
	bool GetRequirementInfo(FGameplayTag& OutObjectiveTag, FName& OutItemID, int32& OutItemAmount) const;
};
