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
 * 다양한 모듈 (GAS, 퀘스트, 플레이어) 등등이 태스크를 등록하고 완료 시에 로딩 페이즈를 마칩니다.
 * 페이즈 방송 시, 각종 작업에 대한 완료를 보장합니다.
 * 
 * @note GameMode의 StartPlay 이전 시점에 태스크 등록을 완료해야 합니다.
 */
UCLASS()
class PROJECTAE_API UGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 게임 페이즈 전환 (인레이드, 로비 등) 시에 방송될 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnGamePhaseChanged OnGamePhaseChangeDelegate;


	// 게임모드가 호출할 게임 페이즈 지정 방송 함수
	void SetGamePhase(FGameplayTag NewPhase);

	// 게임모드가 모든 등록 절차를 끝낸 뒤에 호출합니다
	void StartPhaseMonitoring();

	// 로딩 작업을 등록합니다 (예: QuestSystem)
	void RegisterLoadingTask(FName SystemName);

	// 로딩 작업이 끝났음을 알립니다
	void CompleteLoadingTask(FName SystemName);


private:
	// 현재 페이즈
	FGameplayTag CurrentPhase;

	// 현재 진행 중인 로딩 작업 목록
	TSet<FName> PendingLoadingTasks;

	// GameMode가 등록 끝을 선언했는지 여부
	bool bIsMonitoring = false;

	// 태스크 목록이 비었는지 확인하는 함수
	void CheckLoadingState(); 
};
