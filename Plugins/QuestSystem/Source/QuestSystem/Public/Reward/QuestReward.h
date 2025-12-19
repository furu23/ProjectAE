// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestReward.generated.h"

/**
 * @brief 퀘스트 보상의 추상 기반 클래스입니다.
 * 구체적인 보상 로직(아이템 지급, 경험치 등)은 이 클래스를 상속받아 구현합니다.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestReward : public UObject
{
    GENERATED_BODY()

public:
    /**
     * @brief 실제 보상을 지급하는 함수입니다.
     * @param WorldContextObject 월드 컨텍스트 (Subsystem 접근 용)
     */
    virtual void GiveReward(UObject* WorldContextObject);

    // 에디터에서 보상 목록을 볼 때 표시될 이름 (옵션)
    virtual FString GetDescription() const { return TEXT("Reward Base"); }
};