// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AEInRaidGameMode.generated.h"

/**
 * @brief 레이드 레벨의 게임모드입니다.
 */
UCLASS()
class PROJECTAE_API AAEInRaidGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void StartPlay() override;
	
};
