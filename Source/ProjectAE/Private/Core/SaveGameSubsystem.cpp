// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SaveGameSubsystem.h"
#include "Core/AESaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Core/AEGlobalHelper.h"
#include "GameFramework/Character.h"
#include "Inventory/InventoryComponent.h"
#include "Quest/AEQuestSubSystem.h"


void USaveGameSubsystem::NewGame()
{
	// 퀘스트 시스템 초기화
	UAEQuestSubSystem* QuestSys = UAEGlobalHelper::GetQuestSubsystem(GetWorld());
	if (QuestSys)
	{
		QuestSys->SetupNewGameQuests();
	}

	// 이벤트 배열 명시적 초기화
	LoadedCompletedEvents = FGameplayTagContainer::EmptyContainer;
}

void USaveGameSubsystem::SaveGame(bool bSetAsyncLoad)
{
	UAESaveGame* SaveInst = Cast<UAESaveGame>(UGameplayStatics::CreateSaveGameObject(UAESaveGame::StaticClass()));

	UAEQuestSubSystem* QuestSys = UAEGlobalHelper::GetQuestSubsystem(GetWorld());
	if (QuestSys)
	{
		QuestSys->GetSaveData(SaveInst->QuestSystemData);
	}

	// 플레이어 인벤토리 세이브 로직
	// 1. 현재 살아있는 플레이어가 있다면 캐시 최신화
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Character)
	{
		UInventoryComponent* InventoryComponent = Character->FindComponentByClass<UInventoryComponent>();
		if (InventoryComponent && InventoryComponent->bIsPlayerInventory)
		{
			InventoryComponent->GetSaveData(PlayerInventoryCache);
			UE_LOG(LogTemp, Warning, TEXT("SaveGameSubsystem : PlayerInventory Cache Updated from Live Actor."));
		}
	}

	// 2. 캐시 데이터를 세이브 인스턴스에 기록
	if (PlayerInventoryCache.Num() > 0)
	{
		SaveInst->PlayerInventoryData = PlayerInventoryCache;
	}
	
	/* 추가 세이브 로직을 여기에 */
	
	// 이벤트 항목 저장
	SaveInst->CompletedEvents = LoadedCompletedEvents;

	if (bSetAsyncLoad)
	{
		UGameplayStatics::AsyncSaveGameToSlot(SaveInst, FString("Save"), 0);
	}
	else
	{
		UGameplayStatics::SaveGameToSlot(SaveInst, FString("Save"), 0);
	}
}

void USaveGameSubsystem::LoadGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(FString("Save"), 0)) return;

	UAESaveGame* LoadInst = Cast<UAESaveGame>(UGameplayStatics::LoadGameFromSlot(FString("Save"), 0));
	if (!LoadInst) return;

	UAEQuestSubSystem* QuestSys = UAEGlobalHelper::GetQuestSubsystem(GetWorld());
	if (QuestSys)
	{
		QuestSys->PreLoadGame(LoadInst->QuestSystemData);
	}

	// 캐시 로드
	PlayerInventoryCache = LoadInst->PlayerInventoryData;
	UE_LOG(LogTemp, Warning, TEXT("SaveGameSubsystem : PlayerInventory Cache Loaded from Disk."));

	// 만약 현재 게임 중이라면 즉시 적용
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Character)
	{
		UInventoryComponent* InventoryComponent = Character->FindComponentByClass<UInventoryComponent>();
		if (InventoryComponent && InventoryComponent->bIsPlayerInventory)
		{
			InventoryComponent->LoadSaveData(PlayerInventoryCache);
			UE_LOG(LogTemp, Warning, TEXT("SaveGameSubsystem : PlayerInventory Live Actor Updated."));
		}
	}
	
	/* 추가 로드 로직을 여기에 */

	// 이베트 항목 로드
	LoadedCompletedEvents = LoadInst->CompletedEvents;
}

void USaveGameSubsystem::SaveInventoryToCache(const TArray<uint8>& Data)
{
	PlayerInventoryCache = Data;
}

bool USaveGameSubsystem::GetInventoryFromCache(TArray<uint8>& OutData)
{
	if (PlayerInventoryCache.Num() == 0) return false;
	
	OutData = PlayerInventoryCache;
	return true;
}

bool USaveGameSubsystem::IsEventCompleted(const FGameplayTagContainer& EventTag) const
{
	if (!EventTag.IsEmpty())
	{
		return false;
	}

	return LoadedCompletedEvents.HasAllExact(EventTag);
}

bool USaveGameSubsystem::IsEventCompleted(const FGameplayTag& EventTag) const
{
	if (!EventTag.IsValid())
	{
		return false;
	}

	return LoadedCompletedEvents.HasTagExact(EventTag);
}

void USaveGameSubsystem::MarkEventCompleted(const FGameplayTagContainer& EventTag)
{
	if (!EventTag.IsEmpty()) return;

	LoadedCompletedEvents.AppendTags(EventTag);
}

void USaveGameSubsystem::MarkEventCompleted(const FGameplayTag& EventTag)
{
	if (!EventTag.IsValid()) return;

	LoadedCompletedEvents.AddTag(EventTag);
}
