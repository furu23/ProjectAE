// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestNotificationWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
class USoundBase;

/**
 * 퀘스트 알림 전용 위젯 베이스 클래스
 */
UCLASS()
class PROJECTAE_API UQuestNotificationWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // **** UI 바인딩 (이름이 일치해야 함) ****
    
    // 알림 텍스트 (예: "쥐 잡기 (1/3)")
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Notification;

    // 등장/퇴장 애니메이션
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    TObjectPtr<UWidgetAnimation> Anim_PopUp;
    
    UPROPERTY(EditAnywhere, Category= "Sound")
    TObjectPtr<USoundBase> NotifySound;

    // **** 초기화 및 GMS ****
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // GMS 메시지 수신 핸들러
    void OnNotificationReceived(FGameplayTag Channel, const FQuestNotificationMessage& Payload);

    // GMS 리스너 핸들
    FGameplayMessageListenerHandle ListenerHandle;
};