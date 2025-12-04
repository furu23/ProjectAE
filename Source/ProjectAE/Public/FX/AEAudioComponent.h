// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "AEAudioComponent.generated.h"

/**
 * @brief 오디오를 감쇠 및 동시성 에셋을 포함해 복합적으로 관리하기 위한 클래스입니다. (현재 미사용)
 * 
 * @note 동시성에 대한 필요가 크게 없기 때문에, 현재 확장을 멈추고 사용중이지 않습니다. 
 */
UCLASS()
class PROJECTAE_API UAEAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UAEAudioComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 외부 트리거용
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void TriggerFootstep();
};
