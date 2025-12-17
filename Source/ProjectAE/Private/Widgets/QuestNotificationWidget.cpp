// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/QuestNotificationWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"

void UQuestNotificationWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // GMS 구독: UI.Event.Notification 태그 리슨
    UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);

    ListenerHandle = GMS.RegisterListener<FQuestNotificationMessage>(
        FGameplayTag::RequestGameplayTag(TEXT("Quest.Event.UI.Notification")), // 네이티브 태그 사용
        this,
        &UQuestNotificationWidget::OnNotificationReceived
    );
}

void UQuestNotificationWidget::NativeDestruct()
{
    if (ListenerHandle.IsValid())
    {
        ListenerHandle.Unregister();
    }
    Super::NativeDestruct();
}

void UQuestNotificationWidget::OnNotificationReceived(FGameplayTag Channel, const FQuestNotificationMessage& Payload)
{
    // 1. 텍스트 갱신
    if (Text_Notification)
    {
        Text_Notification->SetText(Payload.NotificationText);
    }

    // 2. 애니메이션 재생 (처음부터 재생)
    if (Anim_PopUp)
    {
        StopAnimation(Anim_PopUp);
        PlayAnimation(Anim_PopUp);
    }
}