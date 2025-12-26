#include "Action/QuestAction.h"

void UQuestAction::ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestContext& QuestContext)
{
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

void UQuestAction::Native_ExecuteAction(UObject* WorldContext, const FGameplayTag& QuestID, const FQuestContext& QuestContext)
{
	
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
