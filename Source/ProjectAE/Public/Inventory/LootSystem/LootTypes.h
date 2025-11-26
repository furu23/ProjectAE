// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LootTypes.generated.h"


USTRUCT(BlueprintType)
struct FLootTableEntry : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle ItemDataHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinAmount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmount = 1;
};

USTRUCT(BlueprintType)
struct FLootTable : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLootTableEntry> PossibleItems;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinItemTypes = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxItemTypes = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EmptyChance = 0.f;
};