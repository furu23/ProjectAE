// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widgets/InventoryWidget.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Widgets/InventorySlotWidget.h"


void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInventoryWidget::NativeDestruct()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::OnInventoryUpdated);
	}
	
	Super::NativeDestruct();
}

void UInventoryWidget::InitializeInventory(UInventoryComponent* InitInventoryComponent)
{
	if (!InitInventoryComponent || !ItemGrid || !SlotWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryWidget::InitializeInventory: Invalid parameters."));
		return;
	}

	InventoryComponent = InitInventoryComponent;
	InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
	RefreshInventory();
}

void UInventoryWidget::RefreshInventory()
{
	if (!InventoryComponent || !ItemGrid) return;

	ItemGrid->ClearChildren();
	SlotWidgets.Empty();

	for (int32 i = 0; i < InventoryComponent->MaxSlots; i++)
	{
		if (UInventorySlotWidget* SlotWidget = CreateSlotWidget(i))
		{
			SlotWidget->SetupSlot(InventoryComponent, i);
			SlotWidget->UpdateSlot();

			int32 Row = i / ColumnsPerRow;
			int32 Column = i % ColumnsPerRow;
			
			UUniformGridSlot* GridSlot = ItemGrid->AddChildToUniformGrid(SlotWidget, Row, Column);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Center);
				GridSlot->SetVerticalAlignment(VAlign_Center);
			}
			
			SlotWidgets.Add(SlotWidget);
		}
	}
}

void UInventoryWidget::OnInventoryUpdated()
{
	for (UInventorySlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->UpdateSlot();
		}
	}
}

UInventorySlotWidget* UInventoryWidget::CreateSlotWidget_Implementation(int32 SlotIndex)
{
	if (!SlotWidgetClass) return nullptr;

	return CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
}
