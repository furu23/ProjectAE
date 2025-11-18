// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObject.h"
#include "Data/DA_QuestBase.h"
#include "Objectives/Config/QuestObjectiveConfig.h"
#include "Objectives/QuestObjective.h"
#include "QuestSystem.h"

void UQuestObject::Initialize(UDA_QuestBase* DefRef, FQuestProgressData* ProgressRef, UQuestManagerSubSystem* Manager)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] Object Initialize"), *this->GetFName().ToString());
	// 값 초기화
	Definition = DefRef;
	ProgressDataRef = ProgressRef;
	CachedQuestSys = Manager;

	// QuestObjective 배열 초기화
	for (const UQuestObjectiveConfig* Config : DefRef->ObjectConfigs)
	{
		if (!Config) continue;

		TSubclassOf<UQuestObjective> ObjectiveClass = Config->GetQuestObjectiveClass();
		if (!ObjectiveClass)
		{
			UE_LOG(LogQuestSystem, Warning, TEXT("Quest [%s] has an invalid ObjectiveConfig or Class!"), *Definition->GetName());
			continue;
		}

		// UClass 값을 통해 ObjectiveClass를 만들고, 
		UQuestObjective* Objective = NewObject<UQuestObjective>(this, ObjectiveClass);
		Objective->Initialize(Config, ProgressRef);
		Objectives.Add(Objective);
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object initialized end"), *this->GetFName().ToString());
	}
}

void UQuestObject::Activate(UObject* WorldContext)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object activating is started"), *this->GetFName().ToString());
	for (UQuestObjective* Objective : Objectives)
	{
		Objective->OnObjectiveCompleteDelegate.BindUObject(this, &UQuestObject::OnObjectiveCompleted);
		// Objective->OnRequestTaskSignatureDelegate.BindUObject(this, &UAEQuestObject::OnObjectiveRequestingTasks);
		Objective->Activate(this);
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object activated end"), *this->GetFName().ToString());
	}
}

void UQuestObject::DeActivate()
{
	for (UQuestObjective* Objective : Objectives)
	{
		Objective->DeActivate();
		Objective->OnObjectiveCompleteDelegate.Unbind();

		Objective->MarkAsGarbage();
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object deactivating is called successfully"), *this->GetFName().ToString());
	}
	Objectives.Empty();
}

void UQuestObject::OnObjectiveCompleted(UQuestObjective* Objective)
{
	if (CheckQuestCompletion())
	{
		ProgressDataRef->ProgressType = EQuestProgress::Completed_PendingTurnIn;
		DeActivate();
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object get [%s] objective completion"), *this->GetFName().ToString(), *Objective->GetFName().ToString());
	}
	OnQuestObjectChangedDelegate.Execute(Definition->QuestID);
}

bool UQuestObject::CheckQuestCompletion()
{
	for (UQuestObjective* Objective : Objectives)
	{
		if (!Objective->IsComplete()) return false;
	}
	return true;
}

/*
void UAEQuestObject::OnObjectiveRequestingTasks(const TArray<TObjectPtr<UQuestWorldTask>> TasksToExecute)
{
	OnRequestWorldTasksDelegate.ExecuteIfBound(TasksToExecute);
}*/