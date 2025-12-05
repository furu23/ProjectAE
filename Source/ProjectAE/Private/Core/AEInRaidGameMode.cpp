// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AEInRaidGameMode.h"
#include "Core/GamePhaseSubsystem.h"

void AAEInRaidGameMode::StartPlay()
{
    Super::StartPlay();

    // 월드 시스템 가져오기
    UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>();

    // 레이드 상태 선언 및 방송
    PhaseSys->StartPhaseMonitoring();
}