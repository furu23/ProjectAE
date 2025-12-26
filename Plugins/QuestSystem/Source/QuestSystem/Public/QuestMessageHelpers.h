// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "QuestMessageHelpers.generated.h"

struct FQuestMessage_Generic;

/**
 * GMS를 통해 퀘스트 시스템에 메시지를 '방송'하는 정적 헬퍼 클래스입니다.
 */
UCLASS()
class QUESTSYSTEM_API UQuestHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief AI가 죽었을 때 이벤트를 GMS로 방송합니다.
	 * @param WorldContextObject 이벤트를 방송하는 액터 (this)
	 * @param InstigatorActor AI를 죽인 액터 (플레이어 등)
	 * @param TargetActor 죽인 AI 등
	 * @param TargetTags 채널에 담길 목표 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Messaging", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastAIKilledEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor, FGameplayTagContainer TargetTags);

	/**
	 * @brief 플레이어가 특정 대상과 상호작용했을 때 이벤트를 GMS로 방송합니다.
	 * @param WorldContextObject 이벤트를 방송하는 액터 (this)
	 * @param InstigatorActor 상호작용한 액터 (플레이어 등)
	 * @param TargetActor 상호작용한 액터
	 * @param TargetTags 채널에 담길 목표 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Messaging", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastInteractEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor, FGameplayTagContainer TargetTags);

	/**
	 * @brief 플레이어가 특정 지역에 이동했을 때 때 이벤트를 GMS로 방송합니다.
	 * @param WorldContextObject 이벤트를 방송하는 액터 (this)
	 * @param InstigatorActor 지역에 위치한 액터 (플레이어 등)
	 * @param TargetActor 트리거를 촉발시킨 액터
	 * @param TargetTags 채널에 담길 목표 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Messaging", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastLocationEvent(UObject* WorldContextObject, AActor* InstiagtorActor, AActor* TargetActor, FGameplayTagContainer TargetTags);

private:
	// 메시지를 방송하는 내부 래퍼
	static void SendQuestMessage(UObject* WorldContextObject, FGameplayTag Channel, const FQuestMessage_Generic& Message);
};