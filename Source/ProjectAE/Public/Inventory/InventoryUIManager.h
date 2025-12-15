// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryUIManager.generated.h"


/**
 * (구) 인벤토리 위젯-컴포넌트 연결용으로 만들었으나,
 * 현재는 HUD가 위젯을 관리하고 본 컴포넌트는 인벤토리 관련 컴포넌트 간 통신/상태 제어를 담당한다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UInventoryUIManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryUIManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenPlayerInventory();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClosePlayerInventory();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenChestInventory(class UInventoryComponent* ChestInventory);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseChestInventory();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseAllInventories();
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	class UInventoryComponent* GetPlayerInventory() const;
	UFUNCTION(BlueprintPure, Category = "Inventory")
	class UInventoryComponent* GetCurrentChestInventory() const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool QuickMoveItem(class UInventoryComponent* Source, int32 SourceSlotIndex);
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetGameHUDWidget(class UAEGameHUDWidget* Widget);
	
protected:
	UPROPERTY()
	class UInventoryComponent* CachedPlayerInventory;
	UPROPERTY()
	class UInventoryComponent* CurrentOpenChestInventory;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI")
	bool bIsPlayerInventoryOpen = false;
	
	UPROPERTY()
	TObjectPtr<class APlayerController> PC;
	
	UPROPERTY()
	TObjectPtr<class UAEGameHUDWidget> GameHUDWidget;
};
