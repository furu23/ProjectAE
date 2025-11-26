// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjective.h"
#include "QuestObjective_Interact.generated.h"

class UObjectiveConfig_Interact;

/**
 * 
 */
UCLASS()
class PROJECTAE_API UQuestObjective_Interact : public UQuestObjective
{
	GENERATED_BODY()

public:
	// **** 기본 공용 함수 오버라이드 ****

	virtual void Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag QuestID) override;

	virtual void Activate(UObject* WorldContext) override;

	virtual void DeActivate() override;

	virtual bool IsComplete() const override;
	
protected:
	// **** GMS에 바인딩될 함수 오버라이드 ****

	virtual void OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message) override;


	// **** 편의용 Config 객체 변환 ****

	const UObjectiveConfig_Interact* InteractConfig;
};
