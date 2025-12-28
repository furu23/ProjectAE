// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "QuestObject.generated.h"

class UQuestObjectConfig;
class UQuestObjective;
class UQuestTask;
class UQuestComponent;


DECLARE_DELEGATE_OneParam(FOnObjectUpdatedSignature, const FQuestExecutionContext& /*ActionContext*/);

/**
 * @brief 퀘스트 진행에 대한 런타임 객체입니다. InProgresss 상태의 객체들만 이 객체를 가지며, 실제 퀘스트 목표를 관리하는 역할을 맡습니다.
 */
UCLASS(BlueprintType, Blueprintable)
class QUESTSYSTEM_API UQuestObject : public UObject
{
	GENERATED_BODY()

public:
	// FFastQuestArray를 갱신하기 위한 버블업 델리게이트
	FOnObjectUpdatedSignature OnObjectUpdatedDelegate;


#pragma region Getter & Helpers
public:
	UFUNCTION(BlueprintPure, Category = "Quest")
	FORCEINLINE const FGameplayTag& GetQuestID() const { return Definition ? Definition->QuestID : FGameplayTag::EmptyTag; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	FORCEINLINE const UQuestObjectConfig* GetQuestDefinition() const { return Definition; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	FORCEINLINE UQuestComponent* GetOwnerComponent() const { return CachedQuestComp.Get(); }

	/** 현재 퀘스트의 완료 조건을 만족했는지 확인 (외부 조회용) */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool CheckQuestCompletion() const;

	FORCEINLINE const FGameplayTag& GetQuestID() const { return Definition ? Definition->QuestID : FGameplayTag::EmptyTag; }
	FORCEINLINE const UQuestObjectConfig* GetQuestDefinition() const { return Definition; }
#pragma endregion

#pragma region Public Interface
public:
	// 퀘스트 런타임 객체를 초기화합니다.
	bool Initialize(const UQuestObjectConfig* DefRef, UQuestComponent* OwnerComp);
	// Objective 배열을 순회하며 활성화 시킵니다. 델리게이트에 OnObjectCompleted 함수를 바인드합니다.
	void Activate();
	// 모든 배열을 비활성화시킵니다.
	void DeActivate();
#pragma endregion

#pragma region Quest Object Internal Flow
protected:
	void OnObjectiveUpdated(const FQuestExecutionContext& ActionContext);

	virtual bool Native_Initalize(const UQuestObjectConfig* DefRef, UQuestComponent* OwnerComp);
	virtual void Native_Activate();
	virtual void Native_DeActivated();
	virtual bool Native_OnObjectiveUpdated(const UQuestObjective* Objective);

	virtual bool Native_CheckQuestCompletion(bool bAllObjectivesComplete) const;

	// BP Hooks, 내부 로직, Native 함수 뒤에서 실행됩니다. DeActivate의 경우에는 가장 먼저 실행됩니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Object", meta = (DisplayName = "OnInitialize"))
	bool K2_Initialize(const UQuestObjectConfig* DefRef, UQuestComponent* OwnerComp);
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Object", meta = (DisplayName = "OnActivate"))
	void K2_Activate();
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Object", meta = (DisplayName = "OnDeActivate"))
	void K2_DeActivate();
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Object", meta = (DisplayName = "OnObjectiveUpdated"))
	void K2_OnObjectUpdated(const UQuestObjective* Objective);
#pragma endregion

#pragma region Data & State
protected:
	// 이 퀘스트 런타임 객체의 설계도가 되는 UDA_QuestBase 객체에 대한 참조입니다.
	UPROPERTY()
	TObjectPtr<const UQuestObjectConfig> Definition;

	// 퀘스트의 하위 목표 객체 배열입니다.
	UPROPERTY(VisibleAnywhere, Category = "Quest")
	TArray<TObjectPtr<UQuestObjective>> Objectives;

	// 실패 목표입니다.
	UPROPERTY(VisibleAnywhere, Category = "Quest")
	TArray<TObjectPtr<UQuestObjective>> FailObjectives;

	// 퀘스트 컴포넌트에 대한 캐싱입니다. 유효성 검사 필요.
	UPROPERTY()
	TWeakObjectPtr<UQuestComponent> CachedQuestComp;

private:
	bool bIsActive = false;
#pragma endregion

#pragma region DEBUG ONLY
/*
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
public:
	// 디버그 전용 퀘스트 완료 함수입니다.
	void ForceCompleteQuest();

	// 디버그 전용 퀘스트 목표 완료 함수입니다.
	void ForceCompleteQuestObj(const FGameplayTag& ObjectiveID);
#endif*/
#pragma endregion
};