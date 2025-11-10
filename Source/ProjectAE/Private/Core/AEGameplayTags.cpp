#include "Core/AEGameplayTags.h"
#include "GameplayTagsManager.h"

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