// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/QuestObjectConfig.h"
#include "QuestSystem.h"

#if WITH_EDITOR
#include "Objectives/QuestObjectiveConfig.h"
#endif

FPrimaryAssetId UQuestObjectConfig::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return FPrimaryAssetId();
	}

	FName AssetType = FName("QuestData");
	
	FName AssetName = QuestID.GetTagName();
	if (AssetName.IsNone())
	{
		AssetName = NAME_None;
	}

	UE_LOG(LogQuestSystem, Verbose, TEXT("### Registering Asset: %s, ID: %s"), *GetName(), *FPrimaryAssetId(AssetType, AssetName).ToString());
	return FPrimaryAssetId(AssetType, AssetName);
}