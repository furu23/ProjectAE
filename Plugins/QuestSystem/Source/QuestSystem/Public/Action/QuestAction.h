#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "QuestAction.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class QUESTSYSTEM_API UQuestAction : public UObject
{
	GENERATED_BODY()

public:
	// 퀘스트 액션 실행 함수
	UFUNCTION(BlueprintNativeEvent, Category = "Quest Action", meta = (ToolTip = "퀘스트 액션을 실행합니다."))
	void ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID);
	virtual void ExecuteAction_Implementation(UObject* WorldContext, const FGameplayTag& QuestID);
};