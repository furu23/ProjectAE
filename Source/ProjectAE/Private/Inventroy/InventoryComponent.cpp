// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventroy/InventoryComponent.h"

#include "Data/ItemData.h"
#include "Inventroy/Data/InventorySlot.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (!bIsPlayerInventory) return;
	
	// TODO: 게임 인스턴스에서 인벤토리 데이터 받아오기
	// OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::AddItem(FName ItemID, int32 Amount, int32& OutRemainingAmount)
{
	OutRemainingAmount = Amount;
	if (!ItemDataTable || Amount <= 0) return false;

	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, "");
	if (!ItemData) return false;

	int32 RemainingAmount = Amount;

	// 기존 슬롯에 아이템 채우기
	for (FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.GetItemID() == ItemID && Slot.Amount < ItemData->MaxStackSize)
		{
			int32 AvailableSpace = ItemData->MaxStackSize - Slot.Amount;
			int32 AddAmount = FMath::Min(RemainingAmount, AvailableSpace);

			Slot.Amount += AddAmount;
			RemainingAmount -= AddAmount;

			if (RemainingAmount <= 0) break;
		}
	}

	// 빈 슬롯에 아이템 채우기 (새 슬롯 생성)
	while (RemainingAmount > 0 && InventorySlots.Num() < MaxSlots)
	{
		int32 StackAmount = FMath::Min(RemainingAmount, ItemData->MaxStackSize);
		
		FInventorySlot NewSlot;
		NewSlot.ItemDataHandle.DataTable = ItemDataTable;
		NewSlot.ItemDataHandle.RowName = ItemID;
		NewSlot.Amount = StackAmount;

		InventorySlots.Add(NewSlot);
		RemainingAmount -= StackAmount;
	}

	OutRemainingAmount = RemainingAmount;

	// 일부라도 추가되었으면 성공
	bool bSuccess = RemainingAmount < Amount;
	if (bSuccess)
	{
		OnInventoryUpdated.Broadcast();
	}
	
	return bSuccess;
}

bool UInventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	bool bSuccess = false;
	
	if (!ItemDataTable || Amount <= 0) return bSuccess;
	
	int32 CurrentAmount = GetItemAmount(ItemID);
	if (CurrentAmount < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough items to remove: %s (Required: %d, Has: %d)"), 
			*ItemID.ToString(), Amount, CurrentAmount);
		return bSuccess;
	}
	
	// 뒤에서부터 아이템 제거
	int32 RemainingAmount = Amount;
	for (int32 i = InventorySlots.Num() - 1; i >= 0; i--)
	{
		FInventorySlot& Slot = InventorySlots[i];
		if (Slot.GetItemID() == ItemID)
		{
			int32 RemoveAmount = FMath::Min(RemainingAmount, Slot.Amount);
			Slot.Amount -= RemoveAmount;
			RemainingAmount -= RemoveAmount;

			if (Slot.IsEmpty())
			{
				InventorySlots.RemoveAt(i);
			}

			bSuccess = true;
			if (RemainingAmount <= 0) break;
		}
	}

	OnInventoryUpdated.Broadcast();
	return bSuccess;
}

bool UInventoryComponent::MoveItem(UInventoryComponent* Target, int32 FromSlot, int32 ToSlot)
{
	bool bSuccess = false;
	
	if (!Target || IsSlotValid(FromSlot)) return bSuccess;
	if (Target == this) return bSuccess;	// 같은 인벤토리 일 경우에는 Swap 사용

	FInventorySlot& SourceSlot = InventorySlots[FromSlot];
	if (SourceSlot.IsEmpty()) return bSuccess;

	FItemData* ItemData = SourceSlot.GetItemData();
	if (!ItemData) return bSuccess;

	// ToSlot 이 -1 일 경우 자동으로 추가
	if (ToSlot == -1)
	{
		int32 Remaining = 0;
		bSuccess = Target->AddItem(SourceSlot.GetItemID(), SourceSlot.Amount, Remaining);
		if (bSuccess)
		{
			int32 MoveAmount = SourceSlot.Amount - Remaining;
			SourceSlot.Amount = Remaining;

			if (SourceSlot.IsEmpty())
			{
				InventorySlots.RemoveAt(FromSlot);
			}

			OnInventoryUpdated.Broadcast();
			Target->OnInventoryUpdated.Broadcast();
		}

		return bSuccess;
	}

	// ToSlot이 지정된 경우
	if (!Target->IsSlotValid(ToSlot))
	{
		// 목표 슬롯이 없으면 새로 생성
		if (Target->InventorySlots.Num() >= Target->MaxSlots) return bSuccess;
		
		FInventorySlot NewSlot = SourceSlot;
		Target->InventorySlots.Add(NewSlot);
		InventorySlots.RemoveAt(FromSlot);
		
		bSuccess = true;
		OnInventoryUpdated.Broadcast();
		Target->OnInventoryUpdated.Broadcast();
		return bSuccess;
	}

	FInventorySlot& TargetSlot = Target->InventorySlots[ToSlot];
	
	// 목표 슬롯이 비어있으면 그냥 이동
	if (TargetSlot.IsEmpty())
	{
		TargetSlot = SourceSlot;
		InventorySlots.RemoveAt(FromSlot);
		
		bSuccess = true;
		OnInventoryUpdated.Broadcast();
		Target->OnInventoryUpdated.Broadcast();
		return bSuccess;
	}

	// 같은 아이템이면 스택 합치기
	if (TargetSlot.GetItemID() == SourceSlot.GetItemID())
	{
		int32 CanAdd = FMath::Min(SourceSlot.Amount, ItemData->MaxStackSize - TargetSlot.Amount);
		
		if (CanAdd > 0)
		{
			TargetSlot.Amount += CanAdd;
			SourceSlot.Amount -= CanAdd;
			
			if (SourceSlot.IsEmpty())
			{
				InventorySlots.RemoveAt(FromSlot);
			}
			
			bSuccess = true;
			OnInventoryUpdated.Broadcast();
			Target->OnInventoryUpdated.Broadcast();
			return bSuccess;
		}
	}
	
	return bSuccess;
}

void UInventoryComponent::SwapSlots(int32 FromSlot, int32 ToSlot)
{
	if (!IsSlotValid(FromSlot) || !IsSlotValid(ToSlot)) return;
	if (FromSlot == ToSlot) return;

	FInventorySlot& SlotA = InventorySlots[FromSlot];
	FInventorySlot& SlotB = InventorySlots[ToSlot];

	// 같은 아이템이면 스택 합치기
	if (SlotA.GetItemID() == SlotB.GetItemID())
	{
		FItemData* ItemData = SlotA.GetItemData();
		if (ItemData && SlotB.Amount < ItemData->MaxStackSize)
		{
			int32 CanAdd = FMath::Min(SlotA.Amount, ItemData->MaxStackSize - SlotB.Amount);
			SlotB.Amount += CanAdd;
			SlotA.Amount -= CanAdd;
			
			if (SlotA.IsEmpty())
			{
				InventorySlots.RemoveAt(FromSlot);
			}
			
			OnInventoryUpdated.Broadcast();
			return;
		}
	}

	InventorySlots.Swap(FromSlot, ToSlot);
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ClearAll()
{
	InventorySlots.Empty();
	OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::ClearSlot(int32 SlotIndex)
{
	if (!IsSlotValid(SlotIndex)) return false;

	InventorySlots.RemoveAt(SlotIndex);
	OnInventoryUpdated.Broadcast();
	return true;
}

int32 UInventoryComponent::GetItemAmount(FName ItemID) const
{
	int32 TotalAmount = 0;
	for (const FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.GetItemID() == ItemID)
		{
			TotalAmount += Slot.Amount;
		}
	}
	
	return TotalAmount;
}

const struct FInventorySlot& UInventoryComponent::GetSlot(int32 SlotIndex) const
{
	static FInventorySlot EmptySlot;
	
	if (IsSlotValid(SlotIndex))
	{
		return InventorySlots[SlotIndex];
	}
	
	return EmptySlot;
}

bool UInventoryComponent::HasItem(FName ItemID, int32 Amount) const
{
	return GetItemAmount(ItemID) >= Amount;
}

bool UInventoryComponent::IsFull() const
{
	return InventorySlots.Num() >= MaxSlots;
}

bool UInventoryComponent::IsSlotValid(int32 SlotIndex) const
{
	return InventorySlots.IsValidIndex(SlotIndex);
}
