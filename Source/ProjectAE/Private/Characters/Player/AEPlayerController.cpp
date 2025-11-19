// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Player/AEPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Inventory/InventoryUIManager.h"
#include "QuestManagerSubSystem.h"
#include "Core/AEHUD.h"


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
	
	if ((AEHUD = GetHUD<AAEHUD>()))
	{
		AEGameHUDWidget = AEHUD->GameHUDWidget;
		if (!AEGameHUDWidget)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Not Set AEGameHUDWidget !!"));
		}
	}
}


void AAEPlayerController::Cheat_AcceptQuest(const FString& QuestIDName)
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UQuestManagerSubSystem* QuestManager = LP->GetSubsystem<UQuestManagerSubSystem>())
		{
			FGameplayTag QuestID = FGameplayTag::RequestGameplayTag(*QuestIDName);
			QuestManager->AcceptQuest(QuestID); // [ȣ��]
			UE_LOG(LogTemp, Log, TEXT("Cheat: Quest [%s] Accepted."), *QuestIDName);
		}
	}
}