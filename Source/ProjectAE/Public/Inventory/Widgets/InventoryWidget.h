// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory(class UInventoryComponent* InitInventoryComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshInventory();

protected:
	UFUNCTION()
	void OnInventoryUpdated();

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory")
	class UInventorySlotWidget* CreateSlotWidget(int32 SlotIndex);
	virtual UInventorySlotWidget* CreateSlotWidget_Implementation(int32 SlotIndex);
	
public:
	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* ItemGrid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<class UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 ColumnsPerRow = 5;

protected:
	UPROPERTY(BlueprintreadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY()
	TArray<class UInventorySlotWidget*> SlotWidgets;
};
