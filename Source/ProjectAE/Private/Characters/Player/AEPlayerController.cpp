// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Player/AEPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Inventory/InventoryUIManager.h"
#include "Core/AEHUD.h"
#include "Widgets/AEGameHUDWidget.h"

#if UE_BUILD_DEVELOPMENT
#include "Core/AEGloabalHelper.h"
#include "Quest/AEQuestSubSystem.h"
#endif


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

void AAEPlayerController::OnInteractionFocusChanged(AActor* NewFocusedActor)
{
	if (!AEGameHUDWidget) return;
	
	if (NewFocusedActor)
	{
		AEGameHUDWidget->ShowInteractionPrompt();
	}
	else
	{
		AEGameHUDWidget->HideInteractionPrompt();
	}
}

#if UE_BUILD_DEVELOPMENT
void AAEPlayerController::Cheat_AcceptQuest(const FString& QuestIDName)
{
	UAEQuestSubSystem* QuestManager = UAEGloabalHelper::GetQuestSubsystem(this);
	if (!ensureMsgf(QuestManager, TEXT("QuestManager is not valid")))
	{
		const FGameplayTag& QuestID = FGameplayTag::RequestGameplayTag(*QuestIDName);
		QuestManager->AcceptQuestForID(QuestID);
		UE_LOG(LogTemp, Log, TEXT("Cheat: Quest [%s] Accepted."), *QuestIDName);
	}
}
#endif
