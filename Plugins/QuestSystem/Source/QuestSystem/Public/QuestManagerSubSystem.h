// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "Logging/LogMacros.h"
#include "Task/QuestTask.h"
#include "QuestManagerSubSystem.generated.h"


// 전방 선언

class UQuestObject;
class UDA_QuestBase;
struct FStreamableHandle;


// 델리게이트 선언

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestEntryUpdatedSignature, const FQuestLogEntry&, UpdatedEntry);
DECLARE_DELEGATE_OneParam(FOnQuestTaskBubbleSignature, const TArray<TObjectPtr<UQuestTask>>&);


/**
 * @brief In-Raid 레벨 및 로비(창고) 레벨에서 모두 퀘스트를 관리하고 추적할 수 있도록 관리하는 서브시스템입니다.
 * @note 데이터와 이벤트 주도의 방식으로 설계되었으며. 코드의 수정보다는 실제 Quest 행동에 대한 로직은 Gameplay Ability에서, 데이터는 DA에서 변경을 시도해주시길 바랍니다.
 */
UCLASS(Abstract)
class QUESTSYSTEM_API UQuestManagerSubSystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	// **** 초기화 관련 ****

	// 초기화 함수
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;


	// **** 기능을 위한 공용 API 함수 ****

	// UI 초기화 시, 퀘스트 목록을 생성
    UFUNCTION(BlueprintCallable, Category = "Quest", meta = (BlueprintPure = "false", ToolTip = "첫 생성 시 모든 DTO 객체를 빌드하고 보냅니다."))
    TArray<FQuestLogEntry> GetQuestLogEntries() const;



	// **** 기능을 위한 공용 델리게이트 ****

	// UI 에 사용될 Entry 단일 객체를 가져오는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events", meta = (ToolTip = "단일 객체를 업데이트 하는 델리게이트입니다."))
    FOnQuestEntryUpdatedSignature OnQuestEntryUpdatedDelegate;

	// 목표 객체에서 월드 태스크에 요청이 들어왔을 때
	FOnQuestTaskBubbleSignature OnQuestTaskBubbleUpDelegate;

	

	// **** 저장 및 로드용 공용 API ****

	// 저장 시 PlayerQuestHistory 만을 저장합니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "퀘스트 시스템에 필요한 저장 정보를 직렬화 해 저장합니다."))
	void GetSaveData(TArray<uint8>& OutData);

	// PlayerQuestHistory 를 복구합니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "퀘스트 시스템에 필요한 직렬화된 저장 정보를 로드합니다."))
	void LoadSaveData(const TArray<uint8>& InData);



	// **** 내부 시스템용 쿼리 함수 *****

	// QuestID에 해당되는 FQuestProgressData의 값을 질의하고 받습니다
	virtual FQuestProgressData* QueryProgressDataForQuestID(const FGameplayTag& QuestID);


	
	// **** 새 게임 시작 시 호출될 기본 초기화 함수 ****

	// NewGame 시에 호출되는 PlayerQuestHistory 초기 상태 전이 함수입니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "새 게임 시작 시 호출되어야 하는 함수입니다."))
	virtual void SetupNewGameQuests();


protected:
	// **** 퀘스트 상태 추적 프로퍼티 ****

	// 모든 퀘스트들의 추적을 로드 및 저장
	UPROPERTY(SaveGame)
	TMap<FGameplayTag, FQuestProgressData> PlayerQuestHistory;

	// PlayerQuestHistoty를 기반으로 빌드되는 설계도 객체 캐시들 (현재 전부 로드되어 있는 설계입니다 많은 퀘스트 사용 시 변용을 요합니다)
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UDA_QuestBase>> ActiveQuestDACaches;

	// 현재 활성화된 퀘스트 오브젝트들
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UQuestObject>> ActiveQuests;



	// **** 하위클래스 전용 상태 변경 알림 함수 ****

	virtual void NotifyQuestUpdate(const FGameplayTag& QuestID);



	// **** 퀘스트 플로우 관련 ****

	// 퀘스트 수락 함수
	virtual void AcceptQuest(const FGameplayTag& QuestID);
	
	// 퀘스트 보상 함수
	virtual void ClaimQuestReward(const FGameplayTag& QuestID);

	// 내부에서 호출될 실제 보상 수령 함수
	virtual void GiveReward(const FGameplayTag& QuestID);

	// 후속 퀘스트의 상태 변경을 검사하고 실행하는 함수
	virtual void TryUnlockNextQuests(const FGameplayTag& QuestID);



	// **** 활성화 퀘스트 관련 ****

	// 첫 초기화 시에, 현재 InProgress 상태인 ActiveQuests 배열을 채움
	virtual void StartActiveQuests();

	// 필요 시, 현재 ActiveQuests 배열을 전부 비활성화하고 비움
	virtual void StopActiveQuests();



	// **** 데이터 비동기 로드 관련 ****

	// 초기화 시점에 에셋 로드 작업을 시작합니다.
	virtual void StartAsyncLoadData();

	// 비동기 로드를 실행하고 받을 콜백 함수
	virtual void OnQuestDataLoaded();



private:
	// **** Private 내부 상태 변화 함수 모음 ****

	// 퀘스트를 활성화 상태로 변경
	virtual void LoadAndActivateQuest(const FGameplayTag& QuestID);

	// 퀘스트를 비활성화 상태로 변경하고 파괴
	virtual void DeactivateAndDestroyQuest(const FGameplayTag& QuestID);



	// **** 태스크 버블링 용 내부 델리게이트 바인딩 함수 ****
	void OnQuestRequestingWorldTasks(const TArray<TObjectPtr<UQuestTask>>& TasksToExecute);



	// **** DTO 생성 로직 ****

	/**
	 * @brief QuestID를 기반으로 완전한 FQuestLogEntry DTO를 생성.
	 * @param QuestID 퀘스트의 고유 ID
	 * @param OutEntry [Out] 생성된 DTO가 담길 변수
	 * @return DTO 생성 성공 여부
	 */
	virtual bool BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const;



	// **** 특수한 상황을 위한 로드 핸들링 ****

	TSharedPtr<FStreamableHandle> LoadHandle;

};
