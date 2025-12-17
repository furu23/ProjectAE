// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AEGameMode.h"
#include "Core/GamePhaseSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void AAEGameMode::StartPlay()
{
    Super::StartPlay();

    // 월드 시스템 가져오기
    UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>();

    // 로비 상태 선언 및 방송
    if (PhaseSys)
    {
        PhaseSys->StartPhaseMonitoring();
    }
}

void AAEGameMode::BeginPlay()
{   
    Super::BeginPlay();

    // [핵심] 찾을 필요 없이, 그냥 "이 태그 들리면 나한테 알려줘"라고 등록만 함
    UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);

    ExtractionListenerHandle = GMS.RegisterListener<FQuestMessage_Generic>(
        FGameplayTag::RequestGameplayTag("Quest.Event.Extract"), // 탈출구에서 쏘는 태그
        this,
        &AAEGameMode::OnExtractionEvent
    );
}

void AAEGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ExtractionListenerHandle.IsValid())
    {
        ExtractionListenerHandle.Unregister();
    }
    Super::EndPlay(EndPlayReason);
}


void AAEGameMode::OnExtractionEvent(FGameplayTag Channel, const FQuestMessage_Generic& Payload)
{
    AActor* SuccessPlayer = Payload.InstigatorActor;

    UE_LOG(LogTemp, Log, TEXT("GameMode: Extraction Confirmed for %s"), *SuccessPlayer->GetName());

    // 처리가 끝나면 GamePhaseSubsystem에게 페이즈 전환 요청
    UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>();
    if (PhaseSys)
    {
        PhaseSys->SetGamePhase(FGameplayTag::RequestGameplayTag("Game.Phase.PostGame"));
    }
}
