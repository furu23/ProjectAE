// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GamePhaseSubsystem.h"
#include "QuestManagerSubSystem.h"
#include "ProjectAE/ProjectAE.h"

void UGamePhaseSubsystem::SetGamePhase(FGameplayTag NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        OnGamePhaseChangeDelegate.Broadcast(CurrentPhase);
        UE_LOG(LogAECore, Log, TEXT("Game Phase Changed: %s"), *NewPhase.ToString());
    }
}

void UGamePhaseSubsystem::RegisterLoadingTask(FName SystemName)
{
    PendingLoadingTasks.Add(SystemName);
}

void UGamePhaseSubsystem::CompleteLoadingTask(FName SystemName)
{
    PendingLoadingTasks.Remove(SystemName);
    CheckLoadingState();
}

void UGamePhaseSubsystem::CheckLoadingState()
{
    if (PendingLoadingTasks.Num() == 0)
    {
        // 현재 고치는 중...
        SetGamePhase(FGameplayTag::RequestGameplayTag("Game.Phase.Lobby"));
    }
}
