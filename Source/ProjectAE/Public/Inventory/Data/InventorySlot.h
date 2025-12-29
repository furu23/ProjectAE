// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Data/ItemData.h"
#include "InventorySlot.generated.h"


/**
 * FInventorySlot은 인벤토리의 슬롯을 나타내는 구조체로, 아이템 데이터와 수량 정보를 저장
 */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
public:
	/**
	 * FDataTableRowHandle 은 단순히 데이터 테이블 이름과 로우 이름 정보들만 저장하고 있음
	 * 하드 레퍼런스를 방지하고 메모리 최적화 
	 */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "InventorySlot")
	FDataTableRowHandle ItemDataHandle;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category = "InventorySlot")
	int32 Amount = 0;

	FORCEINLINE bool IsEmpty() const { return Amount <= 0 || ItemDataHandle.IsNull(); }
	FORCEINLINE FName GetItemID() const { return ItemDataHandle.RowName; }
	FORCEINLINE FItemData* GetItemData() const { return ItemDataHandle.GetRow<FItemData>(""); }
};
