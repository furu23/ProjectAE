// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryUIManager.h"

#include "Inventory/InventoryComponent.h"
#include "Widgets/AEGameHUDWidget.h"


// Sets default values for this component's properties
UInventoryUIManager::UInventoryUIManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UInventoryUIManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	PC = GetOwner<APlayerController>();
	if (!PC || !PC->GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryUIManager must be attached to a PlayerController !"));
		return;
	}
	
	UInventoryComponent* PlayerInventory = PC->GetPawn()->FindComponentByClass<UInventoryComponent>();
	if (PlayerInventory && PlayerInventory->bIsPlayerInventory)
	{
		CachedPlayerInventory = PlayerInventory;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerCharacter must have a UInventoryComponent with bIsPlayerInventory set to true !"));
	}
}

void UInventoryUIManager::OpenPlayerInventory()
{
	if (bIsPlayerInventoryOpen) return;
	
	if (!GameHUDWidget || !CachedPlayerInventory) return;
	
	GameHUDWidget->ShowPlayerInventory();
	bIsPlayerInventoryOpen = true;
	
	// 마우스 커서 변경 ?
}

void UInventoryUIManager::ClosePlayerInventory()
{
	if (!bIsPlayerInventoryOpen) return;
	
	if (!GameHUDWidget) return;
	
	GameHUDWidget->HidePlayerInventory();
	bIsPlayerInventoryOpen = false;
	
	// 위젯 표시 때 바뀐 설정 복구
}

void UInventoryUIManager::OpenChestInventory(UInventoryComponent* ChestInventory)
{
	if (!ChestInventory) return;
	if (!GameHUDWidget) return;
	
	// 이미 다른 상자 인벤토리가 열려있을 경우
	CloseChestInventory();
	
	// Player Inventory 먼저 띄우기
	OpenPlayerInventory();
	
	GameHUDWidget->ShowChestInventory(ChestInventory);
	CurrentOpenChestInventory = ChestInventory;
}

void UInventoryUIManager::CloseChestInventory()
{
	if (!CurrentOpenChestInventory) return;
	if (!GameHUDWidget) return;
	
	GameHUDWidget->HideChestInventory();
	CurrentOpenChestInventory->OnInventoryClosed.Broadcast();
	CurrentOpenChestInventory = nullptr;
	
}

void UInventoryUIManager::CloseAllInventories()
{
	ClosePlayerInventory();
	CloseChestInventory();
}

bool UInventoryUIManager::QuickMoveItem(UInventoryComponent* Source, int32 SourceSlotIndex)
{
	if (!Source || !Source->IsSlotValid(SourceSlotIndex)) return false;

	UInventoryComponent* Target = GetCurrentChestInventory();
	if (!Target) return false;
	
	if (Source == GetCurrentChestInventory())
	{
		Target = GetPlayerInventory();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Target: %s"), *Target->GetName());
	return Source->MoveItem(Target, SourceSlotIndex, -1);
}

void UInventoryUIManager::SetGameHUDWidget(class UAEGameHUDWidget* Widget)
{
	if (!Widget) return;
	GameHUDWidget = Widget;
	
	if (!CachedPlayerInventory) return;
	GameHUDWidget->InitializePlayerInventory(CachedPlayerInventory);
}

UInventoryComponent* UInventoryUIManager::GetCurrentChestInventory() const
{
	return CurrentOpenChestInventory;
}

UInventoryComponent* UInventoryUIManager::GetPlayerInventory() const
{
	return CachedPlayerInventory;
}

