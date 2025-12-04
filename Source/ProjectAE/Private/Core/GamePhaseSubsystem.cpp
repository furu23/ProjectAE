// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GamePhaseSubsystem.h"
#include "QuestManagerSubSystem.h"
#include "ProjectAE/ProjectAE.h"

void UGamePhaseSubsystem::RegisterLoadingTask(FName SystemName)
{
    if (bIsMonitoring)
    {
            // 이미 모니터링이 시작된 후에 태스크가 등록되었음을 경고
        UE_LOG(LogAECore, Warning, TEXT("Warning: Task [%s] registered AFTER monitoring started."), *SystemName.ToString());
    }

    PendingLoadingTasks.Add(SystemName);
    UE_LOG(LogAECore, Log, TEXT("[GamePhase] Task Registered: %s. Total: %d"), *SystemName.ToString(), PendingLoadingTasks.Num());
}

void UGamePhaseSubsystem::CompleteLoadingTask(FName SystemName)
{
    if (PendingLoadingTasks.Contains(SystemName))
    {
        PendingLoadingTasks.Remove(SystemName);
        UE_LOG(LogAECore, Log, TEXT("[GamePhase] Task Completed: %s. Remaining: %d"), *SystemName.ToString(), PendingLoadingTasks.Num());

            // 작업이 하나 끝날 때마다 상태 체크
        CheckLoadingState();
    }
}

void UGamePhaseSubsystem::StartPhaseMonitoring()
{
    UE_LOG(LogAECore, Log, TEXT("[GamePhase] GameMode finished registration. Starting Monitoring..."));

    bIsMonitoring = true;

      // 만약 등록된 태스크가 하나도 없었거나, 이미 다 끝난 상태일 수 있으므로 즉시 체크
    CheckLoadingState();
}

void UGamePhaseSubsystem::CheckLoadingState()
{
    if (!bIsMonitoring)
    {
        return;
    }

    if (PendingLoadingTasks.Num() == 0)
    {
        bIsMonitoring = false;

        SetGamePhase(FGameplayTag::RequestGameplayTag("Game.Phase.InGame"));
    }
}

void UGamePhaseSubsystem::SetGamePhase(FGameplayTag NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        OnGamePhaseChangeDelegate.Broadcast(NewPhase);
        UE_LOG(LogAECore, Log, TEXT("[GamePhase] Phase Changed to: %s"), *NewPhase.ToString());
    }
}