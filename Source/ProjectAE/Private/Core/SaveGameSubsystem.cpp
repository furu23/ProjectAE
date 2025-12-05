// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SaveGameSubsystem.h"
#include "Core/AESaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Core/AEGloabalHelper.h"
#include "GameFramework/Character.h"
#include "Inventory/InventoryComponent.h"
#include "Quest/AEQuestSubSystem.h"


void USaveGameSubsystem::SaveGame()
{
	UAESaveGame* SaveInst = Cast<UAESaveGame>(UGameplayStatics::CreateSaveGameObject(UAESaveGame::StaticClass()));

	UAEQuestSubSystem* QuestSys = UAEGloabalHelper::GetQuestSubsystem(GetWorld());
	if (QuestSys)
	{
		QuestSys->GetSaveData(SaveInst->QuestSystemData);
	}

	// 플레이어 인벤토리 세이브 로직
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Character)
	{
		UInventoryComponent* InventoryComponent = Character->FindComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{
			InventoryComponent->GetSaveData(SaveInst->PlayerInventoryData);
			UE_LOG(LogTemp, Warning, TEXT("SaveGameSubsystem : PlayerInventory GetSaveData."));
		}
	}
	/* 추가 세이브 로직을 여기에 */

	UGameplayStatics::AsyncSaveGameToSlot(SaveInst, FString("Save"), 0);
}

void USaveGameSubsystem::LoadGame()
{
	UAESaveGame* LoadInst = Cast<UAESaveGame>(UGameplayStatics::LoadGameFromSlot(FString("Save"), 0));

	UAEQuestSubSystem* QuestSys = UAEGloabalHelper::GetQuestSubsystem(GetWorld());
	if (QuestSys)
	{
		QuestSys->LoadSaveData(LoadInst->QuestSystemData);
	}

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Character)
	{
		UInventoryComponent* InventoryComponent = Character->FindComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{
			InventoryComponent->LoadSaveData(LoadInst->PlayerInventoryData);
			UE_LOG(LogTemp, Warning, TEXT("SaveGameSubsystem : PlayerInventory LoadSaveData."));
		}
	}
	
	/* 추가 로드 로직을 여기에 */
}
