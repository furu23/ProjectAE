// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AEGameMode.h"
#include "Core/GamePhaseSubsystem.h"

void AAEGameMode::StartPlay()
{
    Super::StartPlay();

    // 월드 시스템 가져오기
    UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>();

    // 로비 상태 선언 및 방송
    if (PhaseTag.IsValid())
    {
        PhaseSys->SetGamePhase(PhaseTag);
    }
}