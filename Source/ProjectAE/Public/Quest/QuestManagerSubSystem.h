// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "AEQuestTypes.h"
#include "QuestManagerSubSystem.generated.h"

class UAEQuestObject;
class UDA_QuestBase;

/**
 * @brief In-Raid 레벨 및 로비(창고) 레벨에서 모두 퀘스트를 관리하고 추적할 수 있도록 관리하는 서브시스템입니다.
 * 
 * @note 데이터와 이벤트 주도의 방식으로 설계되었으며. 코드의 수정보다는 실제 Quest 행동에 대한 로직은 Gameplay Ability에서, 데이터는 DA에서 변경을 시도해주시길 바랍니다.
 */
UCLASS()
class PROJECTAE_API UQuestManagerSubSystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:

	// **** 퀘스트 상태 추적 프로퍼티 ****

	// 현재 '진행중인' 퀘스트들을 로드
	UPROPERTY(Transient)
	TMap<FGameplayTag, FQuestProgressData> PlayerQuestHistory;
	
	// 모든 퀘스트들의 정보를 로드 (UI 사용을 위해)
 	UPROPERTY(SaveGame)
 	TArray<TObjectPtr<UDA_QuestBase>> LoadedAllQuests;

	// **** 로비 레벨 관련 ****

	UFUNCTION()
	virtual void OnPlayerLogin();

	UFUNCTION()
	virtual void AcceptQuest();


	virtual void ClaimQuestReward();


	// **** In-Raid 레벨 관련 ****

	UFUNCTION()
	virtual void OnRaidStart();

	UFUNCTION()
	virtual void OnRaidEnd();

private:
	
	// **** Private 내부 상태 변화 함수 모음 ****

	// 퀘스트를 활성화 상태로 변경
	virtual void LoadAndActivateQuest(FGameplayTag QuestID, FQuestProgressData* ProgressData);

	// 퀘스트를 비활성화 상태로 변경하고 파괴
	virtual void DeactivateAndDestroyQuest(UAEQuestObject* QuestObject);


	// **** 퀘스트 계층 참조 빌드 관련 ****

};
