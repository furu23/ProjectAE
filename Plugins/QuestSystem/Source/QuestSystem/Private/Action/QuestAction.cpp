#include "Action/QuestAction.h"

void UQuestAction::ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext)
{
	if (!CanExecuteAction(WorldContext, QuestID, QuestContext))
	{
		OnQuestActionEndedDelegate.Broadcast(this, EQuestActionResult::Fail);
		return;
	}

	CachedWorldContext = WorldContext;
	OwningQuestID = QuestID;
	SavedContext = QuestContext;

	Native_ExecuteAction(CachedWorldContext.Get(), OwningQuestID, SavedContext);
	K2_ExecuteAction(CachedWorldContext.Get(), OwningQuestID, SavedContext);

	if (InstancingPolicy == EActionInstancingPolicy::InstancedPerObject && bAutoDestroyOnActionEnd)
	{
		EndAction();
	}
}

void UQuestAction::Native_ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext)
{
	
}

bool UQuestAction::CanExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext)
{
	if (!QuestID.IsValid() || !QuestContext.IsValid())
	{
		return false;
	}

	// 초기화 시점 확인
	if (QuestContext.bIsRestoring && !bExecuteOnInitialLoad)
	{
		return false;
	}

	return Native_CanExecuteAction(WorldContext, QuestID, QuestContext) && K2_CanExecuteAction(WorldContext, QuestID, QuestContext);
}

bool UQuestAction::Native_CanExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestExecutionContext& QuestContext)
{
	return true;
}

void UQuestAction::EndAction()
{
	K2_EndAction();
	Native_EndAction();

	OnQuestActionEndedDelegate.Broadcast(this);
	OnQuestActionEndedDelegate.Clear();
}

void UQuestAction::Native_EndAction()
{

}

class UWorld* UQuestAction::GetWorld() const
{
	if (CachedWorldContext.IsValid())
	{
		return CachedWorldContext.Get()->GetWorld();
	}

	Super::GetWorld();
}
