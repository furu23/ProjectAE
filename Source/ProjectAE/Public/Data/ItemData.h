// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemData.generated.h"


/**
 * @brief Represents data associated with an item, including its display information, description, icon, and stack size limit.
 *
 * FItemData is used to define individual item properties for gameplay systems.
 * This structure inherits from FTableRowBase, making it suitable for use in data tables.
 */
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	int32 MaxStackSize = 1;
};
