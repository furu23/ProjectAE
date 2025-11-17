// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/DA_QuestBase.h"

FPrimaryAssetId UDA_QuestBase::GetPrimaryAssetId() const
{
	FName AssetType = FName("QuestData");
	
	FName AssetName = QuestID.GetTagName();
	if (AssetName.IsNone())
	{
		ensureMsgf(false, TEXT("QuestID is None in Quest Data Asset: %s"), *GetName());
		AssetName = NAME_None;
	}

	UE_LOG(LogTemp, Log, TEXT("### Registering Asset: %s, ID: %s"), *GetName(), *FPrimaryAssetId(AssetType, AssetName).ToString());
	return FPrimaryAssetId(AssetType, AssetName);
}
