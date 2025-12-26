// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestMessageHelpers.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestTypes.h"
#include "Action/QuestAction.h"


void UQuestHelpers::BroadcastInteractEvent(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor, FGameplayTagContainer TargetTags)
{
	if (!TargetActor) return;

	// 메세지 생성
	FQuestMessage_Generic Message;
	Message.InstigatorActor = InstigatorActor;
	Message.TargetActor = TargetActor;
	Message.TargetTags = TargetTags;

	// 매세지 보내기
	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Quest.Event.Interact"));

	SendQuestMessage(WorldContextObject, Channel, Message);
}


void UQuestHelpers::BroadcastLocationEvent(UObject* WorldContextObject, AActor* InstiagtorActor, AActor* TargetActor, FGameplayTagContainer TargetTags)
{
	if (!TargetActor) return;

	// 메세지 생성
	FQuestMessage_Generic Message;
	Message.InstigatorActor = InstiagtorActor;
	Message.TargetActor = TargetActor;
	Message.TargetTags = TargetTags;

	// 매세지 보내기
	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Quest.Event.Location"));

	SendQuestMessage(WorldContextObject, Channel, Message);
}

// --- Private 내부 래퍼 ---

void UQuestHelpers::SendQuestMessage(UObject* WorldContextObject, FGameplayTag Channel, const FQuestMessage_Generic& MessageRef)
{
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(WorldContextObject);

	GMS.BroadcastMessage(Channel, MessageRef);
}