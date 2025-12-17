// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Reward/QuestReward.h"
#include "GameplayTagContainer.h"
#include "QuestReward_EventTag.generated.h"

/**
 * 
 */
UCLASS()
class UQuestReward_EventTag : public UQuestReward
{
	GENERATED_BODY()

public:
	// **** 기본 오버라이드 함수 ****

	virtual void GiveReward(UObject* WorldContextObject) override;

	virtual FString GetDescription() const override;

protected:
	UPROPERTY(EditInstanceOnly, Category = "Reward|Event", meta = (ToolTip = "저장될 이벤트 태그입니다."))
	FGameplayTag EventTag;
};
