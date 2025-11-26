// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RandomItemSpawnComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API URandomItemSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URandomItemSpawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/**
     * 지정된 인벤토리에 랜덤 아이템 생성
     * @param TargetInventory 아이템을 추가할 대상 인벤토리
     */
    UFUNCTION(BlueprintCallable, Category = "Loot")
    void SpawnItemsIntoInventory(class UInventoryComponent* TargetInventory);
    
    /**
     * 랜덤 아이템 생성 (인벤토리에 추가하지 않고 결과만 반환)
     * @return 생성된 아이템 슬롯 배열
     */
    UFUNCTION(BlueprintCallable, Category = "Loot")
    TArray<struct FInventorySlot> GenerateRandomLoot();

private:
	FRandomStream RandomStream;

	/**
     * Weighted Random으로 아이템 선택
     * @param Entries 선택 가능한 아이템 목록
     * @return 선택된 아이템 엔트리 (실패 시 nullptr)
     */
    struct FLootTableEntry* SelectRandomEntry(TArray<struct FLootTableEntry>& Entries);
    
    /**
     * LootTableHandle에서 실제 LootTable 데이터 가져오기
     * @return LootTable 포인터 (실패 시 nullptr)
     */
    struct FLootTable* GetLootTable() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FDataTableRowHandle LootTableHandle;

};
