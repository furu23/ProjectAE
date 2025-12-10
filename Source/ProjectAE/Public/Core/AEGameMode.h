// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AEGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTAE_API AAEGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void StartPlay() override;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void OnExtractionEvent(FGameplayTag Channel, const FQuestMessage_Generic& Payload);
	
	UPROPERTY(EditDefaultsOnly, Category = "AECore", meta = (ToolTip = "현재 레벨의 태그입니다."))
	FGameplayTag PhaseTag;

private:
	FGameplayMessageListenerHandle ExtractionListenerHandle;
};
