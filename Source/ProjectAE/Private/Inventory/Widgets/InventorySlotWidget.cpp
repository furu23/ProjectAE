// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widgets/InventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/ItemData.h"
#include "Inventory/Data/InventoryDragDropOperation.h"
#include "Inventory/Data/InventorySlot.h"


void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && !IsEmpty())
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	// Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!InventoryComponent || IsEmpty()) return;

	// DragDropOperation 생성
	if (UInventoryDragDropOperation* DragOp = NewObject<UInventoryDragDropOperation>())
	{
		DragOp->SourceInventory = InventoryComponent;
		DragOp->SourceSlotIndex = SlotIndex;
		DragOp->DefaultDragVisual = this;
		DragOp->Pivot = EDragPivot::MouseDown;

		OutOperation = DragOp;
		
		OnDragStarted();
	}
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (!DragOp || !InventoryComponent) return false;

	bool bSuccess = false;

	// 같은 인벤토리 내 이동
	if (DragOp->SourceInventory == InventoryComponent)
	{
		InventoryComponent->SwapSlots(DragOp->SourceSlotIndex, SlotIndex);
		bSuccess = true;
	}
	// 다른 인벤토리로 이동
	else
	{
		bSuccess = DragOp->SourceInventory->MoveItem(InventoryComponent, DragOp->SourceSlotIndex, SlotIndex);
	}

	if (bSuccess)
	{
		OnDropSucceeded();
	}
	return bSuccess;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
}

void UInventorySlotWidget::SetupSlot(class UInventoryComponent* InInventoryComponent, int32 InSlotIndex)
{
	InventoryComponent = InInventoryComponent;
	SlotIndex = InSlotIndex;
}

void UInventorySlotWidget::UpdateSlot()
{
	if (!InventoryComponent) return;

	const FInventorySlot& SlotData = InventoryComponent->GetSlot(SlotIndex);
	bool bIsEmpty = SlotData.IsEmpty();

	// Blueprint 이벤트 호출 (비주얼 업데이트)
	OnSlotUpdated(SlotData, bIsEmpty);

	// C++에서도 기본 업데이트 (선택사항 - BP에서 오버라이드 가능)
	if (ItemIcon && !bIsEmpty)
	{
		FItemData* ItemData = SlotData.GetItemData();
		if (ItemData && !ItemData->Icon.IsNull())
		{
			// Async Load Icon
			ItemData->Icon.LoadSynchronous();
			ItemIcon->SetBrushFromTexture(ItemData->Icon.Get());
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	

	if (AmountText && !bIsEmpty)
	{
		if (SlotData.Amount > 1)
		{
			AmountText->SetText(FText::AsNumber(SlotData.Amount));
			AmountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			AmountText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else if (AmountText)
	{
		AmountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UInventorySlotWidget::IsEmpty() const
{
	return !InventoryComponent;
}
