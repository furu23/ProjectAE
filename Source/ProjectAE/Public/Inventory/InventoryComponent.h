// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClosed);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/**
	 * 아이템을 인벤토리에 추가
	 * @param ItemID 추가할 아이템 ID
	 * @param Amount 추가할 개수
	 * @param OutRemainingAmount 추가하지 못한 개수 
	 * @return 성공 여부 (일부라도 추가되면 true)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, int32 Amount, UPARAM(ref) int32& OutRemainingAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Amount = 1);
	
	/**
	 * 다른 인벤토리로 아이템 이동
	 * @param Target 목표 인벤토리
	 * @param FromSlot 출발 슬롯 인덱스
	 * @param ToSlot 목표 슬롯 인덱스 (-1이면 자동으로 빈 슬롯 찾기)
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItem(UInventoryComponent* Target, int32 FromSlot, int32 ToSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapSlots(int32 FromSlot, int32 ToSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearAll();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ClearSlot(int32 SlotIndex);

	/*
	 *	유틸리티 함수들
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemAmount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const struct FInventorySlot& GetSlot(int32 SlotIndex) const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 Amount = 1) const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsFull() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsSlotValid(int32 SlotIndex) const;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	bool bIsPlayerInventory = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	UDataTable* ItemDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
	int32 MaxSlots = 20;

	/**
	 * TArray 가 실제 데이터이며, SaveGame 에 저장될 대상
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<struct FInventorySlot> InventorySlots;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
	
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryClosed OnInventoryClosed;
};
