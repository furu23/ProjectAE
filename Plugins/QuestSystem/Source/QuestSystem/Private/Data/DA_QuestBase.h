// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DA_QuestBase.generated.h"

class UQuestObjectiveConfig;

/**
 * @brief 퀘스트의 기본 속성들을 정의합니다.
 */
UCLASS()
class QUESTSYSTEM_API UDA_QuestBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 각 퀘스트의 고유한 ID입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "각 퀘스트의 고유한 ID입니다."))
	FGameplayTag QuestID;

	// 선행 퀘스트들의 ID 태그를 담아둔 태그 컨테이너입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (DisplayName = "PrerequisiteQuest(선행퀘스트)", ToolTip = "선행 퀘스트들의 ID를 담는 태그 컨테이너입니다."))
	FGameplayTagContainer PrerequisiteQuests;

	/** 추후 UDA_QuestDependencyCache에 옮겨 빌드 타임 맵 생성, DA 분리 후 IDetailCustomization, 혹은 CallInEditor를 통해 시각적 인사이트 제공 */
	// 후행 퀘스트들의 ID 태그를 담아둔 태그 컨테이너입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (DisplayName = "PostrequisiteQuest(선행퀘스트)", ToolTip = "후행 퀘스트들의 ID를 담는 태그 컨테이너입니다."))
	FGameplayTagContainer PostrequisiteQuests;

	// 퀘스트 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "인게임에 보여질 퀘스트 이름입니다."))
	FText QuestName;

	// 퀘스트 설명입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "인게임에 보여지는 퀘스트 설명입니다."))
	FText Description;

	// 퀘스트의 보상입니다.
	// UPROPERTY(...)
	// FRewardData RewardData;

	// 퀘스트의 목표입니다.
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Objective", meta = (ToolTip = "퀘스트 목표의 설정에 대한 배열입니다."))
	TArray<TObjectPtr<UQuestObjectiveConfig>> ObjectConfigs;

	// Primary Asset ID를 반환합니다.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override final;
};
