// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AEHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API AAEHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UAEGameHUDWidget> GameHUDWidget;
};
