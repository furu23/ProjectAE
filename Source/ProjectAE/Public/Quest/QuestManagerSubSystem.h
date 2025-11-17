// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "AEQuestTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "QuestManagerSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestEntryUpdated, const FQuestLogEntry&, UpdatedEntry);

class UAEQuestObject;
class UDA_QuestBase;

/**
 * @brief In-Raid 레벨 및 로비(창고) 레벨에서 모두 퀘스트를 관리하고 추적할 수 있도록 관리하는 서브시스템입니다.
 * @note 데이터와 이벤트 주도의 방식으로 설계되었으며. 코드의 수정보다는 실제 Quest 행동에 대한 로직은 Gameplay Ability에서, 데이터는 DA에서 변경을 시도해주시길 바랍니다.
 */
UCLASS()
class PROJECTAE_API UQuestManagerSubSystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

	friend class UAEQuestObject;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuestLogEntry> GetQuestLogEntries() const;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestEntryUpdated OnQuestEntryUpdated;

protected:

	// **** 퀘스트 상태 추적 프로퍼티 ****

	// 현재 '진행중인' 퀘스트들을 로드
	UPROPERTY(Transient)
	TMap<FGameplayTag, FQuestProgressData> PlayerQuestHistory;
	
	// 모든 퀘스트들의 정보를 로드 (UI 사용을 위해)
 	UPROPERTY(SaveGame)
 	TArray<TObjectPtr<UDA_QuestBase>> LoadedAllQuests;

	// 현재 활성화된 퀘스트 오브젝트들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAEQuestObject>> ActiveQuests;


	// **** 하위클래스 전용 상태 변경 알림 함수 ****

	virtual void NotifyQuestUpdate(FGameplayTag QuestID);


	// **** 로비 레벨 관련 ****

	virtual void OnPlayerLogin();

	virtual void AcceptQuest(FGameplayTag QuestID);

	virtual void ClaimQuestReward(FGameplayTag QuestID);


	// **** In-Raid 레벨 관련 ****

	virtual void OnRaidStart();

	virtual void OnRaidEnd();

private:

	// **** Private 내부 상태 변화 함수 모음 ****

	// 퀘스트를 활성화 상태로 변경
	virtual void LoadAndActivateQuest(FGameplayTag QuestID, FQuestProgressData* ProgressData);

	// 퀘스트를 비활성화 상태로 변경하고 파괴
	virtual void DeactivateAndDestroyQuest(UAEQuestObject* QuestObject);


	// **** 퀘스트 계층 참조 빌드 관련 ****


	// **** DTO 생성 로직 ****

	/** 
	 * @brief QuestID를 기반으로 완전한 FQuestLogEntry DTO를 생성합니다.
	 * @note 에셋 로드 실패 등 정당한 이유로 실패할 수 있습니다.
	 * @param QuestID 퀘스트의 고유 ID
	 * @param OutEntry [Out] 생성된 DTO가 담길 변수
	 * @return DTO 생성 성공 여부
	 */
	bool BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const;

	// (필요시) ProgressData를 직접 받는 오버로딩
	bool BuildQuestLogEntry(const FGameplayTag& QuestID, const FQuestProgressData& ProgressData, FQuestLogEntry& OutEntry) const;

};
