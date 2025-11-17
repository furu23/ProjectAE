// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AEHUD.h"

#include "Inventory/InventoryUIManager.h"
#include "Widgets/AEGameHUDWidget.h"


void AAEHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GameHUDWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UAEGameHUDWidget is null. Check if the UMG class is set in the AEHUD Blueprint or if CreateWidget failed."));
		return;
	}
	
	GameHUDWidget->AddToViewport();
	
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (UInventoryUIManager* Manager = PC->GetComponentByClass<UInventoryUIManager>())
		{
			Manager->SetGameHUDWidget(GameHUDWidget);
		}
	}
	
}
