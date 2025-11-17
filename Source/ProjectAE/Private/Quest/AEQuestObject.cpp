// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/AEQuestObject.h"
#include "Quest/Data/DA_QuestBase.h"
#include "Quest/Data/Objectives/QuestObjectiveConfig.h"
#include "Quest/Objectives/AEQuestObjective.h"

void UAEQuestObject::Initialize(UDA_QuestBase* DefRef, FQuestProgressData* ProgressRef, UQuestManagerSubSystem* Manager)
{
	// 값 초기화
	Definition = DefRef;
	ProgressDataRef = ProgressRef;
	CachedQuestSys = Manager;

	// QuestObjective 배열 초기화
	for (const UQuestObjectiveConfig* Config : DefRef->ObjectConfigs)
	{
		if (!Config) continue;

		TSubclassOf<UAEQuestObjective> ObjectiveClass = Config->GetQuestObjectiveClass();
		if (!ObjectiveClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest [%s] has an invalid ObjectiveConfig or Class!"), *Definition->GetName());
			continue;
		}

		// UClass 값을 통해 ObjectiveClass를 만들고, 
		UAEQuestObjective* Objective = NewObject<UAEQuestObjective>(this, ObjectiveClass);
		Objective->Initialize(Config, ProgressRef);
		Objectives.Add(Objective);
	}
}

void UAEQuestObject::Activate(UObject* WorldContext)
{
	for (UAEQuestObjective* Objective : Objectives)
	{
		Objective->OnObjectiveCompleteDelegate.BindUObject(this, &UAEQuestObject::OnObjectiveCompleted);
		// Objective->OnRequestTaskSignatureDelegate.BindUObject(this, &UAEQuestObject::OnObjectiveRequestingTasks);
		Objective->Activate(this);
	}
}

void UAEQuestObject::DeActivate()
{
	for (UAEQuestObjective* Objective : Objectives)
	{
		Objective->DeActivate();
		Objective->OnObjectiveCompleteDelegate.Unbind();

		Objective->MarkAsGarbage();
	}
	Objectives.Empty();
}

void UAEQuestObject::OnObjectiveCompleted(UAEQuestObjective* Objective)
{
	if (CheckQuestCompletion())
	{
		ProgressDataRef->ProgressType = EQuestProgress::Completed_PendingTurnIn;
		DeActivate();
	}
	OnQuestObjectChangedDelegate.Execute(Definition->QuestID);
}

bool UAEQuestObject::CheckQuestCompletion()
{
	for (UAEQuestObjective* Objective : Objectives)
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