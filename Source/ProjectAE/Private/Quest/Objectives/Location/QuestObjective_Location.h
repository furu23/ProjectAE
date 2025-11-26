// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjective.h"
#include "QuestObjective_Location.generated.h"

class UObjectiveConfig_Location;

/**
 * 
 */
UCLASS()
class PROJECTAE_API UQuestObjective_Location : public UQuestObjective
{
	GENERATED_BODY()

public:
	// **** 기본 공용 함수 오버라이드 ****

	virtual void Initialize(const UQuestObjectiveConfig* Config, UQuestManagerSubSystem* QuestSys, FGameplayTag ObjectQuestID) override;

	virtual void Activate(UObject* WorldContext) override;

	virtual void DeActivate() override;

	virtual bool IsComplete() const override;

protected:
	// **** GMS에 바인딩될 함수 오버라이드 ****

	virtual void OnMessageReceived(FGameplayTag Channel, const FQuestMessage_Generic& Message) override;


	// **** 편의용 Config 객체 변환 ****

	const UObjectiveConfig_Location* InteractConfig;
};
