// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestDeliverComponent.h"

#include "QuestTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryComponent.h"


// Sets default values for this component's properties
UQuestDeliverComponent::UQuestDeliverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UQuestDeliverComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UQuestDeliverComponent::ProcessDeliver(AActor* Caller)
{
	if (!Caller || RequireItemID == NAME_None || RequireAmount <= 0)
	{
		OnQuestDeliverFailed.Broadcast(FText::FromString(TEXT("Invalid Caller or Requirements")));
		return false;
	}
	
	// 인벤토리 확인 후 아이템 제거 로직
	UInventoryComponent* PlayerInventory = Caller->FindComponentByClass<UInventoryComponent>();
	if (!PlayerInventory || !PlayerInventory->bIsPlayerInventory)
	{
		OnQuestDeliverFailed.Broadcast(FText::FromString(TEXT("Invalid PlayerInventory")));
		return false;
	}
	
	if (PlayerInventory->HasItem(RequireItemID, RequireAmount))
	{
		if (PlayerInventory->RemoveItem(RequireItemID, RequireAmount))
		{
			SendDeliverSuccessEvent(Caller);
			OnQuestDeliverSuccess.Broadcast();
		}
		else
		{
			OnQuestDeliverFailed.Broadcast(FText::FromString(TEXT("Failed RemoveItem")));
			return false;
		}
	}
	else
	{
		OnQuestDeliverFailed.Broadcast(FText::FromString(TEXT("Not Enough Required Items Found")));
		return false;
	}
	
	return true;
}

void UQuestDeliverComponent::SendDeliverSuccessEvent(AActor* Caller)
{
	if (!MessageListenTag.IsValid() || !TargetQuestEventTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UQuestDeliverComponent : Not Valid MessageListen or QuestEvent Tag"));
		return;
	}
	
	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);
	
	FQuestMessage_Generic Message;
	Message.TargetActor = Caller;
	Message.TargetTags.AddTag(TargetQuestEventTag);
	
	GMS.BroadcastMessage(MessageListenTag, Message);
}
