#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "QuestTypes.h"
#include "Engine/StreamableManager.h"
#include "QuestComponent.generated.h"

// 전방 선언

class UQuestObject;
class UQuestObjectConfig;
class IConsoleObject;
struct FStreamableHandle;



USTRUCT(BlueprintType)
struct FQuestFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	// 선형 검색을 사용할 임계값 (변경 자유)
	static constexpr int32 LinearSearchThreshold = 50;


private:
	UPROPERTY(SaveGame)
	TArray<FQuestProgressData> QuestProgressItems;

	// 퀘스트 ID -> 인덱스 매핑 캐시
	UPROPERTY(Transient)
	mutable TMap<FGameplayTag, int32> QuestIndexMap;

	UPROPERTY(Transient)
	mutable bool bCacheDirtyFlag = true;


	// **** FFastArraySerializer 인터페이스 재정의 ****

#pragma region Interface: FastArraySerializer
public:
    void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
    void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
#pragma endregion


	// **** 서버 전용 ****

#pragma region Server-Side Operations
public:
	// 진행도를 업데이트 하는 함수 (서버 전용)
	bool UpdateProgressData(FQuestProgressData& InData);
	bool UpdateProgressData(const FGameplayTag& QuestID, EQuestProgress ProgressType);
	bool UpdateProgressData(const FGameplayTag& QuestID, const FGameplayTag& ObjID, int32 NewValue);
	bool UpdateProgressData(const FGameplayTag& QuestID, const FGameplayTag& ObjID, EQuestProgress ProgressType, int32 NewValue);

	// 초기화 및 마이그레이션 통합 함수
	void InitializeFromSaveData(const TSet<FGameplayTag>& ValidQuestTags);
private:
    // 서버 전용 함수입니다. 권위 있는 쪽에서만 호출되어야 합니다.
    void AddItem(const FQuestProgressData& NewItem);

    // 서버 전용 함수입니다. 권위 있는 쪽에서만 호출되어야 합니다.
    bool RemoveItem(const FGameplayTag& InQuestID);
#pragma endregion


	// **** 공용 API 함수 ****

#pragma region Public API
public:
	// 게터
	FORCEINLINE const FQuestProgressData* Find(const FGameplayTag& QuestID) const { return Internal_Find(QuestID); }
	FORCEINLINE const TArray<FQuestProgressData>& GetItems() const { return QuestProgressItems; }

	// 공용 API
	FORCEINLINE int32 Num() const { return QuestProgressItems.Num();}
	FORCEINLINE bool IsEmpty() const { return QuestProgressItems.Num() == 0; }

	void Empty();

	// 범위 기반 for 지원
	auto begin() { return QuestProgressItems.begin(); }
	auto end() { return QuestProgressItems.end(); }

	auto begin() const { return QuestProgressItems.begin(); }
	auto end() const { return QuestProgressItems.end(); }
#pragma endregion


#pragma region Internal Implementation
private:
	/**
	 * @brief QuestID에 해당되는 FQuestProgressData의 값을 질의하고 받습니다
	 *
	 * @note 내부적으로 선형 검색과 맵 검색을 혼합하여 최적의 성능을 도모합니다.
	 *
	 * @param InQuestID 찾고자 하는 퀘스트의 GameplayTag ID
	 * @return FQuestProgressData* 해당 퀘스트의 진행 데이터 포인터 (없으면 nullptr)
	 */
	const FQuestProgressData* Internal_Find(const FGameplayTag& InQuestID) const;

	// Effective C++ 패턴을 적용한 const_cast 패턴
	// 외부 공용 함수에 const를 적용해 더 안전하고 생산성 있는 구현이 가능
	FQuestProgressData* Internal_Find(const FGameplayTag& InQuestID);

	// 캐시 재구축 함수
	void RebuildCache() const;
#pragma endregion
};

// FFastArraySerializer 특수화
template<>
struct TStructOpsTypeTraits<FQuestFastArray> : public TStructOpsTypeTraitsBase2<FQuestFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


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
    // 퀘스트 기록 (서버->클라 동기화 핵심)
    UPROPERTY(Replicated, SaveGame, ReplicatedUsing = "OnRep_PlayerQuestHistory")
    FQuestFastArray QuestProgressList;

    // 현재 활성화된 퀘스트 객체 인스턴스, 서버 전용
    UPROPERTY(Transient)
    TArray<TObjectPtr<UQuestObject>> ActiveQuests;

    UPROPERTY(Transient)
	TArray<TObjectPtr<UQuestAction>> ActiveQuestActions;

    // 퀘스트 데이터 테이블
    UPROPERTY(EditDefaultsOnly, Category = "Quest|Config")
    TObjectPtr<UDataTable> QuestMetadataTable;

    // 메타데이터 캐시
    UPROPERTY(Transient)
    TMap<FGameplayTag, FQuestTableRow> QuestMetadataCache;


    UFUNCTION()
    virtual void OnRep_PlayerQuestHistory(const FQuestFastArray& OldQuestHistory);
#pragma endregion


    // =================================================================
    // 3. Public Queries (UI 및 외부 시스템 조회용 - const 위주)
    // =================================================================
#pragma region Public Queries
public:
    /** * @brief UI 초기화용. 가벼운 요약 정보 목록을 반환합니다. (동기)
     * QuestProgressList + QuestMetadataCache 조합
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
	bool ValidateAcceptQuest(const FGameplayTag& QuestID) const;
	bool ValidateClaimQuestReward(const FGameplayTag& QuestID) const;
	bool ValidateAbandonQuest(const FGameplayTag& QuestID) const;

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

    UFUNCTION(Client, Reliable)
	virtual void Client_AcceptQuest_Rejected(const FGameplayTag& QuestID);
	UFUNCTION(Client, Reliable)
	virtual void Client_ClaimQuestReward_Rejected(const FGameplayTag& QuestID);
    UFUNCTION(Client, Reliable)
	virtual void Client_AbandonQuest_Rejected(const FGameplayTag& QuestID);


	// --- Helpers ---
	// 보상 지급 처리
	void GiveReward(const FGameplayTag& QuestID);
	// 후행 퀘스트 상태 전이 확인
    void TryUnlockNextQuests(const FGameplayTag& QuestID);
    // 정책에 따른 퀘스트 액션 실행
    void ExecuteQuestActions(TArray<TObjectPtr<UQuestAction>>& Actions, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext, ENetworkActionType NetPolicy);

    // --- Callback ---
    // 퀘스트 액션 완료 시 콜백
    void OnQuestActionEnded(UQuestAction* EndedAction);
	// 퀘스트 완료 시 콜백
    void OnQuestObjectCompelete(UQuestObject* CompleteQuest);
    // 목표 변경 시 콜백
    void OnQuestProgressUpdated(UQuestObject* UpdatedQuest, const FGameplayTag& ObjID, int32 NewValue);
#pragma endregion


    // =================================================================
	// System Logic Flow (저장 및 초기화, 비동기 로드)
    // =================================================================
#pragma region System Logic (Save & Async Load)
public:
    UFUNCTION(BlueprintCallable, Category = "Quest|Save")
    void GetSaveData(TArray<uint8>& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Save")
    void LoadSaveData(const TArray<uint8>& InData);

    // 퀘스트 Object 데이터를 미리 로드합니다. 이 구조체를 가지고 있는 동안 로드가 유지됩니다.
    UFUNCTION(BlueprintCallable, Category = "Quest|AsyncLoad")
	FQuestPreloadHandle RequestPreloadQuestData(const FGameplayTag& QuestID);

protected:
	// --- Initialization ---
    // 저장 데이터로부터 퀘스트 데이터 준비
    void PrepareQuestData();
     
    // 메타데이터 테이블 로드
    void LoadQuestMetadataTable();

	// 퀘스트 오브젝트 데이터 비동기 로드
    void LoadQuestObjectData(const FGameplayTag& QuestID, FStreamableDelegate OnLoadComplete);

    // 로드 콜백 처리 함수
    void ProcessPendingCallbacks(const FGameplayTag& QuestID);

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
    TMap<FGameplayTag, TArray<FStreamableDelegate>> PendingLoadCallbacks;
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