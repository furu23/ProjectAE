// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "RaidSessionSubsystem.generated.h"

/**
 * @brief 레이드의 상태를 통해 진입하는 데 사용합니다.
 * @note 현재는 아직 사용처가 없습니다.
 * 
 * 낮/밤, 변화요소, 날씨, 맵 종류 등 다양한 변화를 확인하는 중간 서브시스템입니다.
 */
UCLASS()
class PROJECTAE_API URaidSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 레이드 입장 전 로비에서 호출
    void PrepareEnterRaid(FGameplayTag RaidId);

    // 현재 레이드 정보
    FGameplayTag GetCurrentRaidId() const { return CurrentRaidId; }

    // 지금 들어온 맵이 레이드 맵인지 확인 (MapName이나 Tag로 구분)
    bool IsInRaidLevel() const;

private:
    FGameplayTag CurrentRaidId;
};
