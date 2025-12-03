// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

/**
 * @brief 저장을 행하는 서브시스템입니다.
 * @note UPROPERTY의 SaveGame 지정자를 이용하도록 만들어졌습니다.
 * 
 * SaveGame 지정자를 사용하지 않는다면 말해주세요.
 * 오버로딩 함수를 추가하는 것도 방법입니다.
 */
UCLASS()
class PROJECTAE_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 세이브 게임 용도
	UFUNCTION(BlueprintCallable)
	void SaveGame();

	// 로드 게임 용도
	UFUNCTION(BlueprintCallable)
	void LoadGame();
};
