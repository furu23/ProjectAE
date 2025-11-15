// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/DA_QuestBase.h"

FPrimaryAssetId UDA_QuestBase::GetPrimaryAssetId() const
{
	FName AssetType = UDA_QuestBase::StaticClass()->GetFName();
	
	FName AssetName = QuestID.GetTagName();
	if (AssetName.IsNone())
	{
		ensureMsgf(false, TEXT("QuestID is None in Quest Data Asset: %s"), *GetName());
		AssetName = NAME_None;
	}

	return FPrimaryAssetId(AssetType, AssetName);
}
