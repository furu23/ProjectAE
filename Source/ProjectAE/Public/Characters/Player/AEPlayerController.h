// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AEPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API AAEPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAEPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
public:
	void OnInteractionFocusChanged(AActor* NewFocusedActor);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* InputMappingContext;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class AAEHUD> AEHUD;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UAEGameHUDWidget> AEGameHUDWidget;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UInventoryUIManager> InventoryUIManager;

	UFUNCTION(Exec)
	void Cheat_AcceptQuest(const FString& QuestIDName);
};
