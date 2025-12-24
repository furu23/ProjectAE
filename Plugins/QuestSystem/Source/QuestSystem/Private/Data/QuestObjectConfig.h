// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Reward/QuestReward.h"
#include "QuestObjectConfig.generated.h"


class UQuestObjectiveConfig;
class UQuestAction;


USTRUCT(BlueprintType)
struct FQuestEventActionWrapper
{
    GENERATED_BODY()

    // 이 태그와 일치하는 이벤트가 왔을 때만 실행
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag TriggerTag;

    // 실행할 액션
    UPROPERTY(EditDefaultsOnly, Instanced)
    TObjectPtr<UQuestAction> Action;
};


/**
 * @brief 퀘스트의 기본 속성들을 정의합니다.
 */
UCLASS()
class QUESTSYSTEM_API UQuestObjectConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/* 이사 예정! */

	// 각 퀘스트의 고유한 ID입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "각 퀘스트의 고유한 ID입니다."))
	FGameplayTag QuestID;

	// 선행 퀘스트들의 ID 태그를 담아둔 태그 컨테이너입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (DisplayName = "PrerequisiteQuest(선행퀘스트)", ToolTip = "선행 퀘스트들의 ID를 담는 태그 컨테이너입니다."))
	FGameplayTagContainer PrerequisiteQuests;

	// 퀘스트 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "인게임에 보여질 퀘스트 이름입니다."))
	FText QuestName;

	// 퀘스트 설명입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "인게임에 보여지는 퀘스트 설명입니다."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "뉴 게임 시작 시, CanAccept 상태인 퀘스트 입니다."))
	bool bIsStartingQuest = false;

	/* ---  */

	// 각 퀘스트의 고유한 ID입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base", meta = (ToolTip = "각 퀘스트의 고유한 ID입니다."))
	FGameplayTag QuestID;


	// **** 퀘스트 정책 설정들 ****

	// 포기 가능 여부
    UPROPERTY(EditDefaultsOnly, Category = "Policy")
    bool bCanAbandon = true;

    // 완료 즉시 보상 자동 수령
    UPROPERTY(EditDefaultsOnly, Category = "Policy")
    bool bAutoClaimReward = false;

	// 반복 가능 여부
	UPROPERTY(EditDefaultsOnly, Category = "Policy")
	bool bIsRepeatable = false;

	// 목표 계층 구조 사용 여부
	UPROPERTY(EditDefaultsOnly, Category = "Policy")
	bool bUseObjectiveHierarchy = false;


	// **** 퀘스트 진행 중 실행되는 액션 객체들 ****

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 수락 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestAcceptAction;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 보상 수령 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestClaimRewardAction;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 목표 활성화 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestObjectiveActivationAction;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 목표 완료 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestObjectiveCompletionAction;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 완료 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestCompletionAction;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 실패 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestFailedAction;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action", meta = (ToolTip = "퀘스트 포기 시 실행되는 액션 객체입니다."))
	TArray<TObjectPtr<UQuestAction>> QuestAbandonAction;

	UPROPERTY(EditDefaultsOnly, Category = "Action", meta = (ToolTip = "퀘스트 커스텀 이벤트 발생 시 실행되는 액션 객체입니다."))
	TArray<FQuestEventActionWrapper> QuestGameplayEventAction;


	// **** 퀘스트 목표 설정들 ****

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Objective", meta = (ToolTip = "퀘스트 목표의 설정에 대한 배열입니다."))
	TArray<TObjectPtr<UQuestObjectiveConfig>> ObjectConfigs;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Objective", meta = (ToolTip = "퀘스트 실패 조건 목표의 설정에 대한 배열입니다."))
    TArray<TObjectPtr<UQuestObjectiveConfig>> FailConditions;


	// Primary Asset ID를 반환합니다.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override final;
};
