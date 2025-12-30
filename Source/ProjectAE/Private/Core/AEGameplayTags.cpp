#include "Core/AEGameplayTags.h"
#include "GameplayTagsManager.h"

namespace AbilityTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Ability_BioHealAmount, "Data.BioHealAmount", "바이오 회복 데이터를 전달할 SetByCaller 태그");
}

FAEGameplayTags FAEGameplayTags::GameplayTagsInstance;

void FAEGameplayTags::InitializeNativeTags()
{
	// Example:
	// AddTag(YourTagVariable, TEXT("Tag.Path.Here"));
}

void FAEGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagString, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(TagString, TagComment);
}