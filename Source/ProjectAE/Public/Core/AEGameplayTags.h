#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

namespace AbilityTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Ability_BioHealAmount);
}

/*
* 게임플레이 태그를 중앙에서 관리하는 싱글톤 구조체입니다.
* .ini 파일에 정의된 네이티브 태그를 여기에 추가하고, 게임 전반에서 참조할 수 있도록 합니다.
*/
struct FAEGameplayTags
{
public:
	// 싱글톤 인스턴스에 접근하는 정적 함수
	static const FAEGameplayTags& Get() { return GameplayTagsInstance; }

	
	static void InitializeNativeTags();

	// **** Ability Tags ****
	// .ini 파일에 정의된 네이티브 태그를 여기에 추가합니다.

protected:
	// 태그 추가를 돕는 헬퍼 함수
	static void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagString, const ANSICHAR* TagComment = nullptr);

private:
	// 싱글톤 인스턴스
	static FAEGameplayTags GameplayTagsInstance;
};