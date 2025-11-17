// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AEPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Inventory/InventoryUIManager.h"


AAEPlayerController::AAEPlayerController()
{
	InventoryUIManager = CreateDefaultSubobject<UInventoryUIManager>(TEXT("InventoryUIManager"));
}

void AAEPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController()) return;
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem
		= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}
