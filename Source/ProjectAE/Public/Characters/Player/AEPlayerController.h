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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* InputMappingContext;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UInventoryUIManager> InventoryUIManager;
	
};
