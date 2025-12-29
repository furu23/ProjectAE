// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestManagerSubSystem.h"
#include "AEQuestSubSystem.generated.h"

/**
 * @brief 퀘스트 로드 시점을 보장하고, 퀘스트 
 */
UCLASS()
class PROJECTAE_API UAEQuestSubSystem : public UQuestManagerSubSystem
{
	GENERATED_BODY()

	friend UQuestManagerSubSystem;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "QuestID를 통해 퀘스트를 수락합니다."))
	virtual void AcceptQuestForID(const FGameplayTag& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "QuestID를 통해 퀘스트를 완료하고 필요 시 보상을 받습니다."))
	virtual void CompleteQuestForID(const FGameplayTag& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "로드 시 데이터를 확실히 하기 위한 함수입니다.."))
	virtual void PreLoadGame(const TArray<uint8>& InData);

protected:
	virtual void OnQuestDataLoaded() override;
};
