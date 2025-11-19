// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "Logging/LogMacros.h"
#include "QuestManagerSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestEntryUpdatedDelegate, const FQuestLogEntry&, UpdatedEntry);

class UQuestObject;
class UDA_QuestBase;
struct FStreamableHandle;

/**
 * @brief In-Raid 레벨 및 로비(창고) 레벨에서 모두 퀘스트를 관리하고 추적할 수 있도록 관리하는 서브시스템입니다.
 * @note 데이터와 이벤트 주도의 방식으로 설계되었으며. 코드의 수정보다는 실제 Quest 행동에 대한 로직은 Gameplay Ability에서, 데이터는 DA에서 변경을 시도해주시길 바랍니다.
 */
UCLASS()
class QUESTSYSTEM_API UQuestManagerSubSystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	// **** 초기화 관련 ****

	// 자체 초기화 함수
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// PC의 BeginPlay 타임에 초기화 작업을 시작합니다. 이 함수가 완료된 이후 QuestSystem이 동작합니다.
	virtual void OnSystemReady(FGameplayTag PhaseTag);



	// **** 기능을 위한 공용 API ****

	// UI 초기화 시, 퀘스트 목록을 생성
    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuestLogEntry> GetQuestLogEntries() const;

	// UI 에 사용될 Entry 단일 객체를 가져오는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestEntryUpdatedDelegate OnQuestEntryUpdated;

	// 퀘스트 수락용
	virtual void AcceptQuest(FGameplayTag QuestID);

	

	// **** 저장 및 로드용 공용 API ****

	// 저장 시 PlayerQuestHistory 만을 저장합니다
	UFUNCTION(BlueprintCallable)
	void GetSaveData(TArray<uint8>& OutData);

	// PlayerQuestHistory 를 복구합니다
	void LoadSaveData(const TArray<uint8>& InData);



	// **** 내부 시스템용 쿼리 함수 *****

	// QuestID에 해당되는 FQuestProgressData의 값을 질의하고 받습니다.
	virtual FQuestProgressData* QueryProgressDataForQuestId(const FGameplayTag& QuestId);



protected:
	// **** 퀘스트 상태 추적 프로퍼티 ****

	// 현재 '진행중인' 퀘스트들을 로드
	UPROPERTY(SaveGame)
	TMap<FGameplayTag, FQuestProgressData> PlayerQuestHistory;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UDA_QuestBase>> ActiveQuestDACaches;

	// 현재 활성화된 퀘스트 오브젝트들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestObject>> ActiveQuests;



	// **** 하위클래스 전용 상태 변경 알림 함수 ****

	virtual void NotifyQuestUpdate(const FGameplayTag& QuestID);



	// **** 로비 레벨 관련 ****

//	virtual void AcceptQuest(FGameplayTag QuestID);

	virtual void ClaimQuestReward(FGameplayTag QuestID);



	// **** In-Raid 레벨 전환 관련 ****

	virtual void OnRaidStart();

	virtual void OnRaidEnd();



private:
	// **** Private 내부 상태 변화 함수 모음 ****

	// 퀘스트를 활성화 상태로 변경
	virtual void LoadAndActivateQuest(FGameplayTag QuestID);

	// 퀘스트를 비활성화 상태로 변경하고 파괴
	virtual void DeactivateAndDestroyQuest(UQuestObject* QuestObject);



	// **** 태스크 버블링 용 내부 델리게이트 바인딩 함수 ****

	// void OnQuestRequestingWorldTasks(const TArray<TObjectPtr<UQuestWorldTask>> TasksToExecute);



	// **** DTO 생성 로직 ****

	/**
	 * @brief QuestID를 기반으로 완전한 FQuestLogEntry DTO를 생성.
	 * @param QuestID 퀘스트의 고유 ID
	 * @param OutEntry [Out] 생성된 DTO가 담길 변수
	 * @return DTO 생성 성공 여부
	 */
	bool BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const;



	// **** 내부 캡슐화된 함수 ****

	// OnSystemReady 함수에서 비동기 로드를 실행하고 받을 콜백 함수
	virtual void OnQuestDataLoaded();



	// **** 특수한 상황을 위한 로드 핸들링 ****

	TSharedPtr<FStreamableHandle> LoadHandle;

};
