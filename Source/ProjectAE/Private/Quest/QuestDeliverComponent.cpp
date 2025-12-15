// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestDeliverComponent.h"

#include "QuestTypes.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Core/AEGloabalHelper.h"
#include "Inventory/InventoryComponent.h"
#include "Slate/SGameLayerManager.h"


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
	
	APlayerCharacter* Player = Cast<APlayerCharacter>(Caller);
	if (!Player)
	{
		OnQuestDeliverFailed.Broadcast(FText::FromString(TEXT("Invalid Caller or Requirements")));
		return false;
	}
	
	// 인벤토리 확인 후 아이템 제거 로직
	UInventoryComponent* PlayerInventory = Player->GetInventoryComponent();
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

	UAEGloabalHelper::BroadcastDeliverEvent(this, nullptr, Caller, TargetQuestEventTag);
}
