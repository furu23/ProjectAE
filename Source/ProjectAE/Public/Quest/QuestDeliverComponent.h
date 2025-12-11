// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "QuestDeliverComponent.generated.h"


// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestDeliverSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestDeliverFailed, FText, FailReason);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UQuestDeliverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestDeliverComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	/**
	 * @brief 상호작용 시 호출되어 퀘스트 아이템 전달/완료 조건을 처리하는 함수
	 * @param Caller 일반적으로 Player 
	 */
	UFUNCTION(BlueprintCallable)
	bool ProcessDeliver(AActor* Caller);
	
protected:
	// 조건이 충족될 시 아이템 제거 후 호출되어 GMS 메시지 발송
	void SendDeliverSuccessEvent(AActor* Caller);
	
public:
	// Quest 전달 성공 시 외부에 알리는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnQuestDeliverSuccess OnQuestDeliverSuccess;
	
	// Quest 전달 실패 시 외부에 알리는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnQuestDeliverFailed OnQuestDeliverFailed;
	
	UPROPERTY(EditAnywhere, Category="Quest")
	FGameplayTag TargetQuestEventTag;
	
	UPROPERTY(EditAnywhere, Category="Quest")
	FGameplayTag MessageListenTag;
	
	UPROPERTY(EditAnywhere, Category="Quest")
	FName RequireItemID = NAME_None;
	
	UPROPERTY(EditAnywhere, Category="Quest")
	int32 RequireAmount = -1;
	
};
