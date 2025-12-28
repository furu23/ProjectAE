// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/QuestObject.h"
#include "Data/QuestObjectConfig.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "Objectives/QuestObjective.h"
#include "QuestSystem.h"
#include "QuestComponent.h"

#pragma region Object Flow : Initialize
bool UQuestObject::Initialize(const UQuestObjectConfig* DefRef, UQuestComponent* OwnerComp)
{
	if (!DefRef) return false;
	if (!OwnerComp) return false;
	if (!DefRef->QuestID.IsValid()) return false;

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] Object Initialize"), *this->GetFName().ToString());
	// 값 초기화
	Definition = DefRef;
	CachedQuestComp = OwnerComp;

	// QuestObjective 배열 초기화
	for (const UQuestObjectiveConfig* Config : DefRef->ObjectConfigs)
	{
		if (!Config) continue;

		TSubclassOf<UQuestObjective> ObjectiveClass = Config->ObjectiveRuntimeClass;
		if (!ObjectiveClass)
		{
			UE_LOG(LogQuestSystem, Warning, TEXT("Quest [%s] has an invalid ObjectiveConfig or Class!"), *Definition->GetName());
			continue;
		}

		// UClass 값을 통해 ObjectiveClass를 만들고, 초기화
		UQuestObjective* Objective = NewObject<UQuestObjective>(this, ObjectiveClass);
		Objective->Initialize(Config, CachedQuestComp.Get(), Definition->QuestID);

		Objective->OnObjectiveUpdatedDelegate.BindUObject(this, &UQuestObject::OnObjectiveUpdated);

		Objectives.Add(Objective);
		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object initialized end"), *this->GetFName().ToString());
	}

	return Native_Initalize(DefRef, OwnerComp) && K2_Initialize(DefRef, OwnerComp);
}

bool UQuestObject::Native_Initalize(const UQuestObjectConfig* DefRef, UQuestComponent* OwnerComp)
{
	return true;
}
#pragma endregion

#pragma region Object Flow : Activate
void UQuestObject::Activate()
{
	if (bIsActive) return;
	if (!CachedQuestComp.IsValid()) return;

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object activating is started"), *this->GetFName().ToString());
	bIsActive = true;

	for (UQuestObjective* Objective : Objectives)
	{
		Objective->Activate(this);

		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object activated end"), *this->GetFName().ToString());
	}
	
	Native_Activate();
	K2_Activate();
}

void UQuestObject::Native_Activate()
{

}
#pragma endregion

#pragma region Object Flow : DeActivate
void UQuestObject::DeActivate()
{
	Native_DeActivated();
	K2_DeActivate();

	for (UQuestObjective* Objective : Objectives)
	{
		Objective->OnObjectiveUpdatedDelegate.Unbind();
		Objective->DeActivate();

		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object deactivating is called successfully"), *this->GetFName().ToString());
	}
	Objectives.Empty();

	// 완료 시
	OnQuestCompletionMetDelegate.ExecuteIfBound(this);
}

void UQuestObject::Native_DeActivated()
{

}
#pragma endregion

#pragma region Object Flow : Callback And CompletionCheck
void UQuestObject::OnObjectiveUpdated(const FQuestExecutionContext& ActionContext)
{
	if (OnQuestProgressChangedDelegate.IsBound())
	{
		OnQuestProgressChangedDelegate.Execute(this, Objective->GetQuestObjectiveID(), Objective->GetCurrentValue());
	}

	if (CheckQuestCompletion())
	{
		if (OnQuestCompletionMetDelegate.IsBound())
		{
			OnQuestCompletionMetDelegate.Execute(this);
		}
	}
}

bool UQuestObject::Native_OnObjectiveUpdated(const UQuestObjective* Objective)
{
	return true;
}

bool UQuestObject::CheckQuestCompletion() const
{
	bool bAllObjectivesComplete = true;
	for (UQuestObjective* Objective : Objectives)
	{
		if (!Objective->IsComplete())
		{
			bAllObjectivesComplete = false;
			break;
		}
	}
	return Native_CheckQuestCompletion(bAllObjectivesComplete);
}

bool UQuestObject::Native_CheckQuestCompletion(bool bAllObjectivesComplete) const
{
	return bAllObjectivesComplete;
}
#pragma endregion


/*
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
void UQuestObject::ForceCompleteQuest()
{
	// ProgressData를 받아옵니다.
	FQuestProgressData* ProgressData = CachedQuestComp.Get()->QueryProgressDataForQuestID(Definition->QuestID);
	if (!ProgressData)
	{
		UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] : [%s] object failed getting FQuestProgressData"), *this->GetFName().ToString());
		return;
	}

	ProgressData->ProgressType = EQuestProgress::Completed_PendingTurnIn;
	DeActivate();

	OnQuestObjectChangedDelegate.Execute(Definition->QuestID);
}


void UQuestObject::ForceCompleteQuestObj(const FGameplayTag& ObjectiveID)
{
	for (UQuestObjective* Objective : Objectives)
	{
		if (!Objective)
		{
			UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] ForceCompleteQuestObj: not valid runtime object. Please Accept Quest First"));
		}

		if(Objective->GetQuestObjectiveID() == ObjectiveID)
		{
			Objective->ForceCompleteQuestObjective();
			break;
		}
	}
}
#endif*/