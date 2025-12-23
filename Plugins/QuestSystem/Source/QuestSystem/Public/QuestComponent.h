#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "QuestTypes.h"
#include "QuestComponent.generated.h"

// 전방 선언

class UQuestObject;
class UQuestObjectConfig;
class IConsoleObject;
struct FStreamableHandle;


// 델리게이트 선언

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestEntryUpdatedSignature, const FQuestLogEntry&, UpdatedEntry);
DECLARE_DELEGATE_OneParam(FOnQuestTaskBubbleSignature, const TArray<TObjectPtr<UQuestTask>>&);



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QUESTSYSTEM_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	// **** 초기화 관련 ****

	UQuestComponent();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// **** 기능을 위한 공용 API 함수 ****

	// UI 초기화 시, 퀘스트 목록을 생성
    UFUNCTION(BlueprintCallable, Category = "Quest", meta = (BlueprintPure = "false", ToolTip = "첫 생성 시 모든 DTO 객체를 빌드하고 보냅니다."))
    TArray<FQuestLogEntry> GetQuestLogEntries() const;

	// 활성화된 퀘스트 객체를 QuestID로 검색합니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "현재 활성화된 퀘스트 객체를 QuestID로 검색합니다."))
	const UQuestObject* FindActiveQuest(const FGameplayTag& QuestID) const;
	


	// **** 기능을 위한 공용 델리게이트 ****

	// UI 에 사용될 Entry 단일 객체를 가져오는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events", meta = (ToolTip = "단일 객체를 업데이트 하는 델리게이트입니다."))
    FOnQuestEntryUpdatedSignature OnQuestEntryUpdatedDelegate;

	

	// **** 저장 및 로드용 공용 API ****

	// 저장 시 PlayerQuestHistory 만을 저장합니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "퀘스트 시스템에 필요한 저장 정보를 직렬화 해 저장합니다."))
	void GetSaveData(TArray<uint8>& OutData);

	// PlayerQuestHistory 를 복구합니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "퀘스트 시스템에 필요한 직렬화된 저장 정보를 로드합니다."))
	void LoadSaveData(const TArray<uint8>& InData);



	// **** 내부 시스템용 쿼리 함수 *****

	// QuestID에 해당되는 FQuestProgressData의 값을 질의하고 받습니다
	FQuestProgressData* QueryProgressDataForQuestID(const FGameplayTag& QuestID);


	
	// **** 새 게임 시작 시 호출될 기본 초기화 함수 ****

	// NewGame 시에 호출되는 PlayerQuestHistory 초기 상태 전이 함수입니다
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (ToolTip = "새 게임 시작 시 호출되어야 하는 함수입니다."))
	virtual void SetupNewGameQuests();



	// **** Blueprint Hooks ****

	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (ToolTip = "퀘스트 상태가 변경되었을 때 호출되는 이벤트입니다."))
	void K2_NotifyQuestUpdate(const FGameplayTag& QuestID);

	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (ToolTip = "퀘스트가 수락되었을 때 호출되는 이벤트입니다."))
	void K2_OnQuestAccepted(const FGameplayTag & QuestID);

	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (ToolTip = "퀘스트 보상이 수령되었을 때 호출되는 이벤트입니다."))
	void K2_OnQuestRewardClaimed(const FGameplayTag& QuestID);


protected:
	// 모든 퀘스트들의 추적을 로드 및 저장
	UPROPERTY(Replicated, SaveGame)
	FQuestFastArray PlayerQuestHistory;

	// 현재 활성화된 퀘스트 오브젝트들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestObject>> ActiveQuests;

	// 로드 시 전체 퀘스트 불변 데이터를 저장해두는 캐시
	UPROPERTY(Transient)
    TMap<FGameplayTag, FQuestTableRow> QuestMetadataCache;





	// **** 하위클래스 전용 상태 변경 알림 함수 ****

	virtual void NotifyQuestUpdate(const FGameplayTag& QuestID);
	// **** 퀘스트 상태 추적 프로퍼티 ****




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



	// **** DTO 생성 로직 ****

	// UI 전달용 DTO를 빌드하는 함수
	virtual bool BuildQuestLogEntry(const FGameplayTag& QuestID, FQuestLogEntry& OutEntry) const;


private:
	// **** Private 내부 상태 변화 함수 모음 ****

	// 퀘스트를 활성화 상태로 변경
	void LoadAndActivateQuest(const FGameplayTag& QuestID);

	// 퀘스트를 비활성화 상태로 변경하고 파괴
	void DeactivateAndDestroyQuest(const FGameplayTag& QuestID);



	// **** 태스크 버블링 용 내부 델리게이트 바인딩 함수 ****
	void OnQuestRequestingWorldTasks(const TArray<TObjectPtr<UQuestTask>>& TasksToExecute);



	// ========= DEVELOPMENT-ONLY =========

	// **** 개발자용 private 디버깅 함수 ****

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

	void Cheat_SetupNewGameQuests();
	void Cheat_ForceCompleteQuest(const FString& QuestID);
	void Cheat_ForceCompleteQuestObj(const FString& QuestID, const FString& ObjectiveID);

	void Console_ForceCompleteQuest(const TArray<FString>& Args);
	void Console_ForceCompleteQuestObj(const TArray<FString>& Args);

	TArray<IConsoleObject*> ConsoleCommands;
#endif

}