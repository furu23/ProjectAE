// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/LootSystem/RandomItemSpawnComponent.h"

#include "Inventory/InventoryComponent.h"
#include "Inventory/Data/InventorySlot.h"
#include "Inventory/LootSystem/LootTypes.h"


// Sets default values for this component's properties
URandomItemSpawnComponent::URandomItemSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void URandomItemSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

    if (!LootTableHandle.DataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LootTableHandle has no DataTable assigned!"), *GetOwner()->GetName());
    }
    else if (LootTableHandle.RowName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LootTableHandle has no RowName assigned!"), *GetOwner()->GetName());
    }
	
}

void URandomItemSpawnComponent::SpawnItemsIntoInventory(UInventoryComponent* TargetInventory)
{
	if (!TargetInventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnItemsIntoInventory: Target Inventory is null!"), *GetOwner()->GetName());
        return;
    }
    
    TArray<FInventorySlot> GeneratedLoot = GenerateRandomLoot();
    
    // 생성된 아이템들을 인벤토리에 추가
    for (const FInventorySlot& Slot : GeneratedLoot)
    {
        if (!Slot.IsEmpty())
        {
        	int32 OutAmount = 0;
            TargetInventory->AddItem(Slot.GetItemID(), Slot.Amount, OutAmount);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[%s] Spawned %d item types into inventory"), *GetOwner()->GetName(), GeneratedLoot.Num());
}

TArray<FInventorySlot> URandomItemSpawnComponent::GenerateRandomLoot()
{
	TArray<FInventorySlot> Result;
    
    // LootTable 가져오기
    FLootTable* LootTable = GetLootTable();
    if (!LootTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get LootTable from Handle"), *GetOwner()->GetName());
        return Result;
    }
    
    // PossibleItems가 비어있는지 확인
    if (LootTable->PossibleItems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LootTable has no PossibleItems!"), *GetOwner()->GetName());
        return Result;
    }
    
    // Random Stream 초기화 (Random Seed 설정)
    RandomStream.Initialize(FMath::Rand());
    
    // 빈 상자 체크
    if (RandomStream.FRand() < LootTable->EmptyChance)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] Chest is empty (EmptyChance: %.2f)"), *GetOwner()->GetName(), LootTable->EmptyChance);
        return Result;
    }
    
    // 스폰할 아이템 종류 개수 결정
    int32 NumItemTypes = RandomStream.RandRange(
        LootTable->MinItemTypes, 
        LootTable->MaxItemTypes
    );
    
    NumItemTypes = FMath::Min(NumItemTypes, LootTable->PossibleItems.Num());
    UE_LOG(LogTemp, Log, TEXT("[%s] Generating %d item types"), *GetOwner()->GetName(), NumItemTypes);
    
    TArray<FLootTableEntry> AvailableEntries = LootTable->PossibleItems;
    
    // 아이템 생성
    for (int32 i = 0; i < NumItemTypes; i++)
    {
        if (AvailableEntries.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] No more available entries"), *GetOwner()->GetName());
            break;
        }
        
        // Weighted Random으로 아이템 선택
        FLootTableEntry* SelectedEntry = SelectRandomEntry(AvailableEntries);
        if (!SelectedEntry)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to select random entry"), *GetOwner()->GetName());
            continue;
        }
        
        // ItemDataHandle 유효성 검증
        if (!SelectedEntry->ItemDataHandle.DataTable || SelectedEntry->ItemDataHandle.RowName.IsNone())
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid ItemDataHandle in LootTableEntry"), *GetOwner()->GetName());
            continue;
        }
        
        // 수량 결정
        int32 Amount = RandomStream.RandRange(
            SelectedEntry->MinAmount, 
            SelectedEntry->MaxAmount
        );
        
        // Slot 생성
        FInventorySlot NewSlot;
        NewSlot.ItemDataHandle = SelectedEntry->ItemDataHandle;
        NewSlot.Amount = Amount;
        
        Result.Add(NewSlot);
        
        UE_LOG(LogTemp, Log, TEXT("[%s] Added item: %s x%d"), 
               *GetOwner()->GetName(), 
               *SelectedEntry->ItemDataHandle.RowName.ToString(), 
               Amount);
    }
    
    return Result;
}

FLootTable* URandomItemSpawnComponent::GetLootTable() const
{
    // DataTable이 설정되지 않음
    if (!LootTableHandle.DataTable)
    {
        return nullptr;
    }
    
    // RowName이 설정되지 않음
    if (LootTableHandle.RowName.IsNone())
    {
        return nullptr;
    }
    
    // DataTable에서 Row 찾기
    FLootTable* Table = LootTableHandle.DataTable->FindRow<FLootTable>(
        LootTableHandle.RowName, 
        TEXT("URandomItemSpawnComponent::GetLootTable")
    );
    
    if (!Table)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Row '%s' not found in LootTable DataTable"), 
            *GetOwner()->GetName(),
            *LootTableHandle.RowName.ToString());
    }
    
    return Table;
}

FLootTableEntry* URandomItemSpawnComponent::SelectRandomEntry(TArray<FLootTableEntry>& Entries)
{
    if (Entries.Num() == 0)
    {
        return nullptr;
    }
    
    // 총 가중치 계산
    float TotalWeight = 0.0f;
    for (const FLootTableEntry& Entry : Entries)
    {
        TotalWeight += Entry.Weight;
    }
    
    if (TotalWeight <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Total weight is zero or negative!"), *GetOwner()->GetName());
        return &Entries[0]; // Fallback: 첫 번째 반환
    }
    
    // 랜덤 값 생성
    float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);
    
    // Weighted Selection
    float CurrentWeight = 0.0f;
    for (int32 i = 0; i < Entries.Num(); i++)
    {
        CurrentWeight += Entries[i].Weight;
        if (RandomValue <= CurrentWeight)
        {
            return &Entries[i];
        }
    }
    
    // Fallback (부동소수점 오차 대비)
    return &Entries.Last();
}

