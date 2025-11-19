// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "GamePhaseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, FGameplayTag, NewPhase);

/**
 * @brief 게임 페이즈를 방송하는 월드시스템입니다.
 * 
 * 다양한 모듈 (GAS, 퀘스트, 플레이어) 등등이 이 델리게이트를 구독해서
 * 레벨의 전환과 그에 맞는 작업을 시작합니다.
 */
UCLASS()
class PROJECTAE_API UGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnGamePhaseChanged OnGamePhaseChangeDelegate;

	void SetGamePhase(FGameplayTag NewPhase);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	FGameplayTag CurrentPhase;
};
