// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestObjective.h"
#include "QuestObjectiveConfig.generated.h"

class UQuestObjective;
class UQuestTask;

/**
 * @brief 퀘스트 목표의 설계도를 정의하는 데이터 에셋 클래스입니다.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestObjectiveConfig : public UObject
{
	GENERATED_BODY()
	
public:
    UQuestObjectiveConfig() 
    {
        ObjectiveRuntimeClass = UQuestObjective::StaticClass();
    }

    
    UPROPERTY(EditAnywhere, Category = "Config")
    FGameplayTag ObjectiveID;

    UPROPERTY(EditAnywhere, Category = "Condition")
    FGameplayTag ListenEvent;

    UPROPERTY(EditAnywhere, Category = "Condition")
    FGameplayTagQuery TargetQuery;

    UPROPERTY(EditAnywhere, Category = "Condition")
    int32 Count = 1;

    UPROPERTY(EditAnywhere, Category = "Condition")
    int32 Priority = 0;
    
    UPROPERTY(EditDefaultsOnly, Category = "System")
    TSubclassOf<UQuestObjective> ObjectiveRuntimeClass;
};