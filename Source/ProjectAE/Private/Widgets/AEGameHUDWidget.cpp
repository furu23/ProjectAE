// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AEGameHUDWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Inventory/Widgets/InventoryWidget.h"
#include "Widgets/Interaction/InteractionPromptWidget.h"


void UAEGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	ChestInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UAEGameHUDWidget::InitializePlayerInventory(class UInventoryComponent* InventoryComponent)
{
	if (PlayerInventoryWidget && InventoryComponent)
	{
		PlayerInventoryWidget->InitializeInventory(InventoryComponent);
		// 플레이어 인벤토리 컴포넌트 캐싱 ?
	}
}

void UAEGameHUDWidget::ShowPlayerInventory()
{
	if (!PlayerInventoryWidget) return;
	
	PlayerInventoryWidget->SetVisibility(ESlateVisibility::Visible);
	PlayerInventoryWidget->RefreshInventory();
}

void UAEGameHUDWidget::HidePlayerInventory()
{
	if (!PlayerInventoryWidget) return;
	
	PlayerInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UAEGameHUDWidget::ShowChestInventory(class UInventoryComponent* ChestInventoryComponent)
{
	if (ChestInventoryWidget && ChestInventoryComponent)
	{
		ChestInventoryWidget->InitializeInventory(ChestInventoryComponent);
		
		InteractionPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
		ChestInventoryWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UAEGameHUDWidget::HideChestInventory()
{
	if (!ChestInventoryWidget) return;
	
	ChestInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	InteractionPromptWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UAEGameHUDWidget::ShowInteractionPrompt()
{
	GetWorld()->GetTimerManager().SetTimer(InteractionPromptTimerHandle, this, &UAEGameHUDWidget::SetInteractionPromptPos, InteractionPromptDelay, true);
	
	InteractionPromptWidget->PlayShowAnimation();
}

void UAEGameHUDWidget::HideInteractionPrompt()
{
	InteractionPromptWidget->PlayHideAnimation();
	
	GetWorld()->GetTimerManager().ClearTimer(InteractionPromptTimerHandle);
}

void UAEGameHUDWidget::SetInteractionPromptPos()
{
	FWidgetTransform WidgetTransform;
	WidgetTransform.Translation = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
	InteractionPromptWidget->SetRenderTransform(WidgetTransform);
}
