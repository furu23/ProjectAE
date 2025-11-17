// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void PlayShowAnimation();
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayHideAnimation();
};
