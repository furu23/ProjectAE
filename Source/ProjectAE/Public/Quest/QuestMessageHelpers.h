// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "QuestMessageHelpers.generated.h"

struct FQuestMessage_Generic;

/**
 * GMS를 통해 퀘스트 시스템에 메시지를 '방송'하는 정적 헬퍼 클래스입니다.
 * 퀘스트 시스템 외부의 모든 액터(AI, 아이템 등)는 이 클래스만 사용해야 합니다.
 */
UCLASS()
class PROJECTAE_API UQuestMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief AI가 죽었을 때 이벤트를 GMS로 방송합니다.
	 * @param WorldContextObject 이벤트를 방송하는 액터 (this)
	 * @param InstigatorActor AI를 죽인 액터 (플레이어 등)
	 * @param TargetActor 죽은 AI 액터
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Messaging", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastAIKilledEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor);

	/**
	 * @brief 플레이어가 특정 대상과 상호작용했을 때 이벤트를 GMS로 방송합니다.
	 * @param WorldContextObject 이벤트를 방송하는 액터 (this)
	 * @param InstigatorActor 상호작용한 액터 (플레이어 등)
	 * @param TargetActor 상호작용의 대상이 된 액터 (퀘스트 아이템, NPC 등)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Messaging", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastInteractEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor, FGameplayTagContainer TargetTags);

private:
	// 메시지를 방송하는 내부 래퍼
	static void SendQuestMessage(UObject* WorldContextObject, FGameplayTag Channel, const FQuestMessage_Generic& Message);
};