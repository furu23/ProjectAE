#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "QuestAction.generated.h"

class UQuestAction;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQuestActionEndedSignature, UQuestAction*, EQuestActionResult);

UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTSYSTEM_API UQuestAction : public UObject
{
	GENERATED_BODY()

public:
	// 퀘스트 액션 실행 함수 Native -> K2 순서로 실행됩니다.
	void ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext);
	// 퀘스트 액션 validation 함수
	bool CanExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext);

	// 퀘스트 액션 종료를 알리는 델리게이트
	FOnQuestActionEndedSignature OnQuestActionEndedDelegate;


	// 게터, Const T 반환
	FORCEINLINE const FGameplayTag& GetOwnerTag() const { return OwningQuestID; }
	// 컨텍스트 반환, 인스턴싱 정책이 NonInstanced 일 때 nullptr
	FORCEINLINE const FQuestExecutionContext* GetContextInfo() const { return InstancingPolicy == EActionInstancingPolicy::NonInstanced ? nullptr : &SavedContext; }

	// 설정 Getter
	FORCEINLINE ENetworkActionType GetNetworkActionType() const { return NetworkActionType; }
	FORCEINLINE EActionInstancingPolicy GetInstancingPolicy() const { return InstancingPolicy; }
	FORCEINLINE bool ShouldDestoryOnQuestEnd() const { return bPersistentAfterQuest; }


protected:
	// Execute 확장을 위한 구현부
	virtual void Native_ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (ToolTip = "퀘스트 액션 실행 시.", DisplayName = "ExecuteAction"))
	void K2_ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext);

	
	// CanExecute 확장을 위한 구현부
	virtual bool Native_CanExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (ToolTip = "퀘스트 액션이 사용가능한지.", DisplayName = "ExecuteAction"))
	bool K2_CanExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext);


	// 퀘스트 액션 종료 함수 K2 -> Native 순서로 실행됩니다.
	void EndAction();
	virtual void Native_EndAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (ToolTip = ".", DisplayName = "EndAction"))
	void K2_EndAction();


	// 편의용 GetWorld 오버라이드
	virtual class UWorld* GetWorld() const override;


private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Policy", meta = (AllowPrivateAccess = "true"))
	ENetworkActionType NetworkActionType = ENetworkActionType::ClientOnly;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Policy", meta = (AllowPrivateAccess = "true"))
	EActionInstancingPolicy InstancingPolicy = EActionInstancingPolicy::NonInstanced;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Policy", meta = (AllowPrivateAccess = "true", EditCondition = "InstancingPolicy != EActionInstancingPolicy::InstancedPerObject", ToolTip = "액션이 끝날 시 자동으로 EndAbility를 호출할 지 여부. (InstancedPerObject일 경우 무시됩니다.)"))
	bool bAutoDestroyOnActionEnd = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Policy", meta = (AllowPrivateAccess = "true", EditCondition = "InstancingPolicy != EActionInstancingPolicy::NonInstanced && bAutoDestroyOnActionEnd", ToolTip = "퀘스트가 완료되어도 자동으로 파괴될 지 여부 (NonInstanced일 경우 무시됩니다.)"))
	bool bPersistentAfterQuest = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Policy", meta = (AllowPrivateAccess = "true", ToolTip = "초기화 시점에 실행할 지 여부를 결정합니다. (ex : OpenLevel)"))
	bool bExecuteOnInitialLoad = false;
	

	// 캐시 용도
    FGameplayTag OwningQuestID;
	FQuestExecutionContext SavedContext;
    TWeakObjectPtr<UObject> CachedWorldContext;
};