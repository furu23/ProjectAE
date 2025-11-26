// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObject.h"
#include "QuestManagerSubSystem.h"
#include "Data/DA_QuestBase.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "Objectives/QuestObjective.h"
#include "QuestSystem.h"

void UQuestObject::Initialize(UDA_QuestBase* DefRef, UQuestManagerSubSystem* Manager)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] Object Initialize"), *this->GetFName().ToString());
	// 값 초기화
	Definition = DefRef;
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
		Objective->Initialize(Config, CachedQuestSys, Definition->QuestID);
		Objectives.Add(Objective);
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object initialized end"), *this->GetFName().ToString());
	}
}

void UQuestObject::Activate(UObject* WorldContext)
{
	UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object activating is started"), *this->GetFName().ToString());
	const FQuestProgressData* QuestData = CachedQuestSys->QueryProgressDataForQuestID(Definition->QuestID);

	for (UQuestObjective* Objective : Objectives)
	{
		Objective->OnObjectiveCompleteDelegate.BindUObject(this, &UQuestObject::OnObjectiveCompleted);
		Objective->OnRequestTaskSignatureDelegate.BindUObject(this, &UQuestObject::OnObjectiveRequestingTasks);
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
		Objective->OnRequestTaskSignatureDelegate.Unbind();

		Objective->MarkAsGarbage();
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object deactivating is called successfully"), *this->GetFName().ToString());
	}
	Objectives.Empty();
}

bool UQuestObject::CheckQuestCompletion()
{
	for (UQuestObjective* Objective : Objectives)
	{
		if (!Objective->IsComplete()) return false;
	}
	return true;
}


void UQuestObject::OnObjectiveCompleted(UQuestObjective* Objective)
{
	if (CheckQuestCompletion())
	{
		// ProgressData를 받아옵니다.
		FQuestProgressData* ProgressData = CachedQuestSys->QueryProgressDataForQuestID(Definition->QuestID);
		if (!ProgressData)
		{
			UE_LOG(LogQuestSystem, Error, TEXT("[QuestSys] : [%s] object failed getting FQuestProgressData"), *this->GetFName().ToString());
			return;
		}

		ProgressData->ProgressType = EQuestProgress::Completed_PendingTurnIn;
		DeActivate();
		UE_LOG(LogQuestSystem, Log, TEXT("[QuestSys] : [%s] object get [%s] objective completion"), *this->GetFName().ToString(), *Objective->GetFName().ToString());
	}
	OnQuestObjectChangedDelegate.Execute(Definition->QuestID);
}


void UQuestObject::OnObjectiveRequestingTasks(const TArray<TObjectPtr<UQuestTask>>& TasksToExecute)
{
	OnRequestWorldTasksDelegate.ExecuteIfBound(TasksToExecute);
}