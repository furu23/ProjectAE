#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "QuestTypes.h"
#include "QuestComponent.generated.h"

// 전방 선언

class UQuestObject;
class UQuestObjectConfig;
class IConsoleObject;
struct FStreamableHandle;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QUESTSYSTEM_API UQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestComponent();

    // =================================================================
    // 1. Core Framework (생명주기 및 리플리케이션)
    // =================================================================
#pragma region Core Framework
public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // 초기화 상태 플래그
    bool bIsInitialized = false;
    bool bQuestsAreActive = false;
    bool bHasLoadedQuestMetadata = false;
    bool bIsDataLoadedFromSave = false;
#pragma endregion


    // =================================================================
    // 2. Data & State (저장되는 데이터와 설정)
    // =================================================================
#pragma region Data and Config
protected:
    // [Replicated] 퀘스트 기록 (서버->클라 동기화 핵심)
    UPROPERTY(Replicated, SaveGame)
    FQuestFastArray PlayerQuestHistory;

    // [Transient] 현재 활성화된 퀘스트 객체 인스턴스
    UPROPERTY(Transient)
    TArray<TObjectPtr<UQuestObject>> ActiveQuests;

    // [Config] 퀘스트 데이터 테이블
    UPROPERTY(EditDefaultsOnly, Category = "Quest|Config")
    TObjectPtr<UDataTable> QuestMetadataTable;

    // [Cache] 메타데이터 캐시
    UPROPERTY(Transient)
    TMap<FGameplayTag, FQuestTableRow> QuestMetadataCache;
#pragma endregion


    // =================================================================
    // 3. Public Queries (UI 및 외부 시스템 조회용 - const 위주)
    // =================================================================
#pragma region Public Queries
public:
    /** * @brief UI 초기화용. 가벼운 요약 정보 목록을 반환합니다. (동기)
     * PlayerQuestHistory + QuestMetadataCache 조합
     */
    UFUNCTION(BlueprintCallable, Category = "Quest|UI")
    TArray<FQuestLogEntry> GetQuestListSummary() const;

    /**
     * @brief 상세 정보를 요청합니다. (비동기)
     * 완료된 퀘스트라면 DA를 로드하고, 진행 중이라면 ActiveQuests에서 정보를 가져옵니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Quest|UI")
    void RequestQuestDetail(FGameplayTag QuestID);

    // 특정 퀘스트 상태 조회
    UFUNCTION(BlueprintCallable, Category = "Quest|Query")
    bool HasActiveQuest(const FGameplayTag& QuestID) const;

	// 특정 퀘스트 오브젝트 조회
    UFUNCTION(BlueprintCallable, Category = "Quest|Query")
    const UQuestObject* FindActiveQuest(const FGameplayTag& QuestID) const;

	// 특정 퀘스트 진행 데이터 조회
    const FQuestProgressData* QueryProgressDataForQuestID(const FGameplayTag& QuestID) const;

    // 상세 로딩 완료 시 UI에 알릴 델리게이트 (1:1 콜백용 델리게이트 정의 필요)
    // FOnQuestDetailLoaded OnQuestDetailLoaded;

    // UPROPERTY(BlueprintAssignable, Category = "Quest|Event")
    // FOnQuestEntryUpdatedSignature OnQuestNotified;
#pragma endregion


    // =================================================================
    // 4. Quest Flow Actions Network (RPC 및 진입점)
    // =================================================================
#pragma region Quest Flow Actions
public:
    // 외부에서 호출하는 진입점 (여기서 로컬 체크 후 Server RPC 호출)
    UFUNCTION(BlueprintCallable, Category = "Quest|Action")
    void RequestAcceptQuest(const FGameplayTag& QuestID);
    UFUNCTION(BlueprintCallable, Category = "Quest|Action")
    void RequestClaimReward(const FGameplayTag& QuestID);
    UFUNCTION(BlueprintCallable, Category = "Quest|Action")
    void RequestAbandonQuest(const FGameplayTag& QuestID);

protected:
    // --- Network RPCs ---
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_AcceptQuest(const FGameplayTag& QuestID);
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ClaimReward(const FGameplayTag& QuestID);
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_AbandonQuest(const FGameplayTag& QuestID);
#pragma endregion


    // =================================================================
	// 4.1. Quest Flow Actions Logic (수락, 진행, 보상)
    // =================================================================
#pragma region Quest Flow Actions Logic
protected:
	// --- NVI Driver ---
	void Internal_AcceptQuest(const FGameplayTag& QuestID);
	void Internal_ClaimQuestReward(const FGameplayTag& QuestID);
	void Internal_AbandonQuest(const FGameplayTag& QuestID);

	// --- Validation ---
	bool CanAcceptQuest(const FGameplayTag& QuestID) const;
	bool CanClaimQuestReward(const FGameplayTag& QuestID) const;
	bool CanAbandonQuest(const FGameplayTag& QuestID) const;

	virtual bool CanAccpetQuest_Native(const FGameplayTag& QuestID) const;
	virtual bool CanClaimQuestReward_Native(const FGameplayTag& QuestID) const;
	virtual bool CanAbandonQuest_Native(const FGameplayTag& QuestID) const;

	// --- Virtual Implementations ---
	virtual void OnPreAcceptQuest(const FGameplayTag& QuestID);
	virtual void OnPreClaimQuestReward(const FGameplayTag& QuestID);
	virtual void OnPreAbandonQuest(const FGameplayTag& QuestID);

	virtual void OnPostAcceptQuest(const FGameplayTag& QuestID);
	virtual void OnPostClaimQuestReward(const FGameplayTag& QuestID);
	virtual void OnPostAbandonQuest(const FGameplayTag& QuestID);

	// --- Core Logic ---
	void AcceptQuest(const FGameplayTag& QuestID);
	void ClaimQuestReward(const FGameplayTag& QuestID);
    void AbandonQuest(const FGameplayTag& QuestID);

       
	// --- Client RPCs ---
	UFUNCTION(Client, Reliable)
	virtual void Client_AcceptQuest(const FGameplayTag& QuestID);
	UFUNCTION(Client, Reliable)
	virtual void Client_ClaimQuestReward(const FGameplayTag& QuestID);
    UFUNCTION(Client, Reliable)
	virtual void Client_AbandonQuest(const FGameplayTag& QuestID);


	// --- Helpers ---
	// 보상 지급 처리
	void GiveReward(const FGameplayTag& QuestID);
	// 후행 퀘스트 상태 전이 확인
    void TryUnlockNextQuests(const FGameplayTag& QuestID);
#pragma endregion


    // =================================================================
	// 5. System Logic Flow (저장 및 초기화, 비동기 로드)
    // =================================================================
#pragma region System Logic (Save & Async Load)
public:
    UFUNCTION(BlueprintCallable, Category = "Quest|Save")
    void GetSaveData(TArray<uint8>& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Save")
    void LoadSaveData(const TArray<uint8>& InData);

protected:
	// --- Initialization ---
    // 저장 데이터로부터 퀘스트 데이터 준비
    void PrepareQuestData();
     
    // 메타데이터 테이블 로드
    void LoadQuestMetadataTable();

    // 비동기 에셋 로드
    void LoadAndActivateQuest(const FGameplayTag& QuestID);
    void OnQuestDataLoaded(const FGameplayTag& QuestID);
    
	// 활성화된 퀘스트 복원
	void RestoreActiveQuest();

	// 모든 활성 퀘스트 중지
	void StopActiveQuests();

	// --- Quest Activation / Deactivation ---
	// 퀘스트 활성화 흐름
	void StartActivateQuest(const FGameplayTag& QuestID);
	void DeactivateAndDestroyQuest(const FGameplayTag& QuestID);

private:
    TMap<FGameplayTag, TSharedPtr<FStreamableHandle>> LoadHandles;
#pragma endregion


    // =================================================================
    // 6. Blueprint Hooks (이벤트)
    // =================================================================
#pragma region Blueprint Hooks
protected:
	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (DisplayName = "OnQuestAccepted"))
    void K2_OnQuestAccepted(const FGameplayTag & QuestID);
	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (DisplayName = "OnQuestRewardClaimed"))
    void K2_OnQuestRewardClaimed(const FGameplayTag& QuestID);
	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (DisplayName = "OnQuestAbandoned"))
	void K2_OnQuestAbandoned(const FGameplayTag& QuestID);

	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (DisplayName = "CanAcceptQuest"))
	bool K2_CanAcceptQuest(const FGameplayTag& QuestID) const;
	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (DisplayName = "CanClaimQuestReward"))
	bool K2_CanClaimQuestReward(const FGameplayTag& QuestID) const;
	UPROPERTY(BlueprintImplementableEvent, Category = "Quest|Events", meta = (DisplayName = "CanAbandonQuest"))
	bool K2_CanAbandonQuest(const FGameplayTag& QuestID) const;
#pragma endregion


    // =================================================================
    // 7. Debug Tools
    // =================================================================
#pragma endregion
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
#pragma region Debug Tools
private:
    void Cheat_ForceCompleteQuest(const FString& QuestID);
    void Cheat_ForceCompleteQuestObj(const FString& QuestID, const FString& ObjectiveID);
    void Console_ForceCompleteQuest(const TArray<FString>& Args);
    void Console_ForceCompleteQuestObj(const TArray<FString>& Args);
    TArray<IConsoleObject*> ConsoleCommands;
#pragma endregion
#endif
};