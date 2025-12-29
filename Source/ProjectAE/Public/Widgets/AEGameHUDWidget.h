// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AEGameHUDWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetOpened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetClosed);

/**
 * 
 */
UCLASS()
class PROJECTAE_API UAEGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	/**
	 * UI 애니메이션 이벤트 (BlueprintImplementable Events)
	 * 블루프린트에서 '이벤트'를 받아 애니메이션 구현
	 */
	// UFUNCTION(BlueprintImplementableEvent, Category = "UI|Interaction")
	// void OnInteractionPromptAnimStart();
	// UFUNCTION(BlueprintImplementableEvent, Category = "UI|Interaction")
	// void OnInteractionPromptAnimStop();
	
public:
	// =========================================================
    // UI 관리 명령 함수 (C++ 구현 - 데이터 로직만 처리)
    // =========================================================
	
	// Inventory Functions
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void InitializePlayerInventory(class UInventoryComponent* InventoryComponent);
	
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void ShowPlayerInventory();
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void HidePlayerInventory();
	
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void ShowChestInventory(class UInventoryComponent* ChestInventoryComponent);
	UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
	void HideChestInventory();
	
	// Interaction Functions
	UFUNCTION(BlueprintCallable, Category = "UI|Interaction")
	void ShowInteractionPrompt();
	
	UFUNCTION(BlueprintCallable, Category = "UI|Interaction")
	void HideInteractionPrompt();
	
	UFUNCTION(BlueprintCallable, Category = "UI|Interaction")
	void SetInteractionPromptPos();
	
	// Quest Functions
	// Quest Widget 은 블루프린트로 구현해 C++ 로 컨버팅할 시간이 없을 것 같아. 그대로 사용하겠습니다.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UI|Quest")
	void ShowQuestPrompt();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UI|Quest")
	void HideQuestPrompt();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UI|Quest")
	void ToggleQuestWidget();
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnWidgetOpened OnWidgetOpened;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnWidgetClosed OnWidgetClosed;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> PlayerInventoryWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> ChestInventoryWidget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UInteractionPromptWidget> InteractionPromptWidget;
	
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<class UUserWidget> QuestPromptWidget;
	
	FTimerHandle InteractionPromptTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Interaction")
	float InteractionPromptDelay = 0.03f;
	
};
