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

void UGamePhaseSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    UQuestManagerSubSystem* QuestSys = GetWorld()->GetFirstPlayerController()->GetLocalPlayer()->GetSubsystem<UQuestManagerSubSystem>();
    if (!QuestSys)
    {
        UE_LOG(LogAECore, Log, TEXT("Failed Get QuestSys"));
        return;
    }
    QuestSys->OnSystemReady(CurrentPhase);
}
