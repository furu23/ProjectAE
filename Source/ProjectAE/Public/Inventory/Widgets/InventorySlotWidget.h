// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetupSlot(class UInventoryComponent* InInventoryComponent, int32 InSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateSlot();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsEmpty() const;

protected:
	// 블루프린트에서 구현할 함수들
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnSlotUpdated(const struct FInventorySlot& SlotData, bool bIsEmpty);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnDragStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnDropSucceeded();

public:
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* AmountText;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex;
};
