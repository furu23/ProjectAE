// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryUIManager.h"

#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Widgets/InventoryWidget.h"


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
	if (!PC)
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
	if (bIsPlayerInventoryOpen || !CachedPlayerInventory) return;
	
	// 위젯 없을 시 생성
	if (!PlayerInventoryWidget && PlayerInventoryWidgetClass)
	{
		PlayerInventoryWidget = CreateWidget<UInventoryWidget>(PC, PlayerInventoryWidgetClass);
		if (PlayerInventoryWidget)
		{
			PlayerInventoryWidget->InitializeInventory(CachedPlayerInventory);
		}
	}
	
	if (PlayerInventoryWidget)
	{
		PlayerInventoryWidget->AddToViewport();
		bIsPlayerInventoryOpen = true;
		
		// TODO 마우스 커서 변경 로직 ?
	}
}

void UInventoryUIManager::ClosePlayerInventory()
{
	if (!bIsPlayerInventoryOpen) return;
	
	if (PlayerInventoryWidget)
	{
		PlayerInventoryWidget->RemoveFromParent();
		bIsPlayerInventoryOpen = false;
		
		// 위젯 표시 때 바뀐 설정 복구
	}
}

void UInventoryUIManager::OpenChestInventory(UInventoryComponent* ChestInventory)
{
	if (!ChestInventory) return;
	
	// 이미 다른 상자 인벤토리가 열려있을 경우
	CloseChestInventory();
	// Player Inventory 먼저 띄우기
	OpenPlayerInventory();
	
	// 위젯 없을 시 생성
	if (!ChestInventoryWidget && ChestInventoryWidgetClass)
	{
		ChestInventoryWidget = CreateWidget<UInventoryWidget>(PC, ChestInventoryWidgetClass);
	}
	
	if (ChestInventoryWidget)
	{
		ChestInventoryWidget->InitializeInventory(ChestInventory);
		ChestInventoryWidget->AddToViewport();
		CurrentOpenChestInventory = ChestInventory;
	}
}

void UInventoryUIManager::CloseChestInventory()
{
	if (!CurrentOpenChestInventory) return;
	
	if (ChestInventoryWidget)
	{
		ChestInventoryWidget->RemoveFromParent();
		CurrentOpenChestInventory = nullptr;
	}
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
	if (Source == GetCurrentChestInventory())
	{
		Target = GetPlayerInventory();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Target: %s"), *Target->GetName());
	return Source->MoveItem(Target, SourceSlotIndex, -1);
}

UInventoryComponent* UInventoryUIManager::GetCurrentChestInventory() const
{
	return CurrentOpenChestInventory;
}

UInventoryComponent* UInventoryUIManager::GetPlayerInventory() const
{
	return CachedPlayerInventory;
}

