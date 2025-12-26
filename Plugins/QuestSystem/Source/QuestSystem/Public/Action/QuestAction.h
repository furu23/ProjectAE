#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "QuestAction.generated.h"

class UQuestAction;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestActionEndedSignature, UQuestAction*);

UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTSYSTEM_API UQuestAction : public UObject
{
	GENERATED_BODY()

public:
	// 퀘스트 액션 실행 함수 Native -> K2 순서로 실행됩니다.
	void ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestContext& QuestContext);
	virtual void Native_ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestContext& QuestContext);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (ToolTip = "퀘스트 액션을 실행합니다.", DisplayName = "ExecuteAction"))
	void K2_ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestContext& QuestContext);


	// 퀘스트 액션 종료 함수 K2 -> Native 순서로 실행됩니다.
	void EndAction();
	virtual void Native_EndAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (ToolTip = ".", DisplayName = "EndAction"))
	void K2_EndAction();


	FOnQuestActionEndedSignature OnQuestActionEndedDelegate;


	UPROPERTY(EditDefaultsOnly, Category = "Policy")
	ENetworkActionType NetworkActionType = ENetworkActionType::ClientOnly;

	UPROPERTY(EditDefaultsOnly, Category = "Policy")
	EActionInstancingPolicy InstancingPolicy = EActionInstancingPolicy::NonInstanced;

	UPROPERTY(EditDefaultsOnly, Category = "Policy", meta = (EditCondition = "InstancingPolicy != EActionInstancingPolicy::InstancedPerObject", ToolTip = "InstancedPerObject일 경우 무시됩니다."))
	bool bAutoDestroyOnActionEnd = true;

	UPROPERTY(EditDefaultsOnly, Category = "Policy", meta = (EditCondition = "InstancingPolicy != EActionInstancingPolicy::NonInstanced && bAutoDestroyOnActionEnd", ToolTip = "NonInstanced일 경우 무시됩니다."))
	bool bPersistentAfterQuest = false;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FGameplayTag OwningQuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FQuestContext SavedContext;

    TWeakObjectPtr<UObject> CachedWorldContext;
};