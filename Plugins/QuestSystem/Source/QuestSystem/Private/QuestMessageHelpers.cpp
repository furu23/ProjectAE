// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestMessageHelpers.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagAssetInterface.h"
#include "QuestTypes.h"
#include "QuestSystem.h"

void UQuestMessageHelpers::BroadcastAIKilledEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor, FGameplayTagContainer TargetTags)
{
	if (!TargetActor) return;

	// 메세지 생성
	FQuestMessage_Generic Message;
	Message.InstigatorActor = InstigatorActor;
	Message.TargetActor = TargetActor;
	Message.Amount = 1;

	// TargetActor에서 태그를 자동으로 가져와 채웁니다.
	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(TargetActor))
	{
		TagInterface->GetOwnedGameplayTags(Message.TargetTags);
	}

	// 메세지 보내기
	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Event.AI.Killed"));
	SendQuestMessage(WorldContextObject, Channel, Message);
}

void UQuestMessageHelpers::BroadcastInteractEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor, FGameplayTagContainer TargetTags)
{
	if (!TargetActor) return;

	// 메세지 생성
	FQuestMessage_Generic Message;
	Message.InstigatorActor = InstigatorActor;
	Message.TargetActor = TargetActor;
	Message.TargetTags = TargetTags;

	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(TargetActor))
	{
		TagInterface->GetOwnedGameplayTags(Message.TargetTags);
	}

	// 매세지 보내기
	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Quest.Event.Interact"));

	SendQuestMessage(WorldContextObject, Channel, Message);
}


void UQuestMessageHelpers::BroadcastLocationEvent(UObject* WorldContextObject, AActor* InstiagtorActor, AActor* TargetActor, FGameplayTagContainer TargetTags)
{
	if (!TargetActor) return;

	// 메세지 생성
	FQuestMessage_Generic Message;
	Message.InstigatorActor = InstiagtorActor;
	Message.TargetActor = TargetActor;
	Message.TargetTags = TargetTags;

	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(TargetActor))
	{
		TagInterface->GetOwnedGameplayTags(Message.TargetTags);
	}

	// 매세지 보내기
	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Quest.Event.Location"));

	SendQuestMessage(WorldContextObject, Channel, Message);
}

// --- Private 내부 래퍼 ---

void UQuestMessageHelpers::SendQuestMessage(UObject* WorldContextObject, FGameplayTag Channel, const FQuestMessage_Generic& MessageRef)
{
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContextObject);

	GMS.BroadcastMessage(Channel, MessageRef);
}