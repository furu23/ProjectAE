// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestObject.h"
#include "QuestManagerSubSystem.h"
#include "Data/QuestData.h"
#include "Objectives/QuestObjectiveConfig.h"
#include "Objectives/QuestObjective.h"
#include "QuestSystem.h"

void UQuestObject::Initialize(UQuestObjectConfig* DefRef, UQuestManagerSubSystem* Manager)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] Object Initialize"), *this->GetFName().ToString());
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
		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object initialized end"), *this->GetFName().ToString());
	}
}

void UQuestObject::Activate(UObject* WorldContext)
{
	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object activating is started"), *this->GetFName().ToString());
	const FQuestProgressData* QuestData = CachedQuestSys->QueryProgressDataForQuestID(Definition->QuestID);

	for (UQuestObjective* Objective : Objectives)
	{
		Objective->OnObjectiveCompleteDelegate.BindUObject(this, &UQuestObject::OnObjectiveCompleted);
		Objective->OnRequestTaskSignatureDelegate.BindUObject(this, &UQuestObject::OnObjectiveRequestingTasks);
		Objective->Activate(this);
		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object activated end"), *this->GetFName().ToString());
	}

	OnQuestObjectChangedDelegate.ExecuteIfBound(Definition->QuestID);
}

void UQuestObject::DeActivate()
{
	for (UQuestObjective* Objective : Objectives)
	{
		Objective->OnObjectiveCompleteDelegate.Unbind();
		Objective->OnRequestTaskSignatureDelegate.Unbind();
		Objective->DeActivate();

		Objective->MarkAsGarbage();
		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object deactivating is called successfully"), *this->GetFName().ToString());
	}
	Objectives.Empty();
}

bool UQuestObject::CheckQuestCompletion() const
{
	for (UQuestObjective* Objective : Objectives)
	{
		if (!Objective->IsComplete()) return false;
	}
	return true;
}


void UQuestObject::ForceCompleteQuest()
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

	OnQuestObjectChangedDelegate.Execute(Definition->QuestID);

}

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

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

#endif

void UQuestObject::OnObjectiveCompleted(UQuestObjective* Objective)
{
	int32 CompletedCount = 0;
	const int32 TotalCount = Objectives.Num();

	for (const UQuestObjective* Obj : Objectives)
	{
		// IsComplete()는 Manager의 ProgressData를 조회하므로 최신 상태를 반영합니다.
		if (Obj && Obj->IsComplete())
		{
			CompletedCount++;
		}
	}

	FQuestNotificationMessage NotiMsg;
	NotiMsg.QuestID = Definition->QuestID;

	// 포맷: "{퀘스트이름} 목표 완료 ({현재}/{전체})"
	// 예시: "마을의 평화 목표 완료 (2/3)"
	NotiMsg.NotificationText = FText::Format(
		NSLOCTEXT("Quest", "ObjectiveCountNotification", "{0} 목표 완료 ({1}/{2})"),
		Definition->QuestName,
		CompletedCount,
		TotalCount
	);

	UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);
	GMS.BroadcastMessage(FGameplayTag::RequestGameplayTag(TEXT("Quest.Event.UI.Notification")), NotiMsg);

	UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] Notification Broadcast: %s (%d/%d)"), *Definition->QuestName.ToString(), CompletedCount, TotalCount);

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
		UE_LOG(LogQuestSystem, Verbose, TEXT("[QuestSys] : [%s] object get [%s] objective completion"), *this->GetFName().ToString(), *Objective->GetFName().ToString());
	}
	OnQuestObjectChangedDelegate.Execute(Definition->QuestID);
}


void UQuestObject::OnObjectiveRequestingTasks(const TArray<TObjectPtr<UQuestTask>>& TasksToExecute)
{
	OnRequestWorldTasksDelegate.ExecuteIfBound(TasksToExecute);
}