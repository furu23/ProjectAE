// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjective.h"
#include "QuestObjective_Kill.generated.h"

class UObjectiveConfig_Kill;

/**
 * @brief 킬 오브젝티브의 런타임 객체 기본 제공형입니다.
 */
UCLASS()
class UQuestObjective_Kill : public UQuestObjective
{
	GENERATED_BODY()

public:
	// **** 기본 공용 함수 오버라이드 ****

	virtual void Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectiveQuestID) override;

	virtual void Activate(UObject* WorldContext) override;

	virtual void DeActivate() override;

	virtual bool IsComplete() const override;

protected:
	// **** GMS에 바인딩될 함수 오버라이드 ****

	virtual void OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message) override;


	// **** 편의용 Config 객체 변환 ****

	const UObjectiveConfig_Kill* KillConfig;
};
