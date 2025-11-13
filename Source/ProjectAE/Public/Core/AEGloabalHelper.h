// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "AEGloabalHelper.generated.h"

// --- 전방 선언 (Forward Declarations) ---

// 1. 프레임워크 클래스
class AAEGameMode;
class AAEGameState;
class AAEPlayerController;
class AAEPlayerState;
class APlayerCharacter;
class AChracter;
class APawn;

// 2. GAS 클래스
class UAEAbilitySystemComponent;
class UAS_BaseCombat;
class UAS_HealthSet;
struct FGameplayEffectContextHandle;

// 3. 서브시스템 클래스
class UQuestManagerSubSystem;
// class UWorldManagerSubsystem;

/**
 * @brief
 * @note 
 */
UCLASS()
class PROJECTAE_API UAEGloabalHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ----------------------------------------------------------------------
	// 1. 프레임워크 접근 헬퍼 (Framework Accessors)
	// (안전한 캐스팅 및 null 체크를 보장합니다)
	// ----------------------------------------------------------------------

	/** 현재 월드의 AAEGameMode를 가져옵니다. (서버 전용) */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Framework", meta = (WorldContext = "WorldContextObject"))
	static AAEGameMode* GetAEGameMode(const UObject* WorldContextObject);

// 	/** 현재 월드의 AAEGameState를 가져옵니다. */
// 	UFUNCTION(BlueprintPure, Category = "MyGame|Framework", meta = (WorldContext = "WorldContextObject"))
// 	static AAEGameState* GetMyGameState(const UObject* WorldContextObject);

	/** 로컬 플레이어의 AAEPlayerController를 가져옵니다. */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Framework", meta = (WorldContext = "WorldContextObject"))
	static AAEPlayerController* GetAEPlayerController(const UObject* WorldContextObject);

	/** 로컬 플레이어의 AAEPlayerState를 가져옵니다. */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Framework", meta = (WorldContext = "WorldContextObject"))
	static AAEPlayerState* GetAEPlayerState(const UObject* WorldContextObject);

	/** 로컬 플레이어의 APlayerCharacter를 가져옵니다. */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Framework", meta = (WorldContext = "WorldContextObject"))
	static APlayerCharacter* GetAECharacter(const UObject* WorldContextObject);

	/** 액터의 소유자(Owner)를 거슬러 올라가 MyPlayerController를 찾습니다. (예: 무기에서 호출) */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Framework")
	static AAEPlayerController* GetOwningPlayerController(UObject* TargetObject);


	// ----------------------------------------------------------------------
	// 2. 서브시스템 접근 헬퍼 (Subsystem Accessors)
	// (GetGameInstance()->GetSubsystem<T>() 호출을 캡슐화합니다)
	// ----------------------------------------------------------------------

	/** UQuestManagerSubSystem(Local Player Subsystem)을 가져옵니다. */
 	UFUNCTION(BlueprintPure, Category = "ProjectAE|Subsystems", meta = (WorldContext = "WorldContextObject"))
 	static UQuestManagerSubSystem* GetQuestSubsystem(const UObject* WorldContextObject);

// 	/** UWorldManagerSubsystem (GameInstance Subsystem)을 가져옵니다. */
// 	UFUNCTION(BlueprintPure, Category = "MyGame|Subsystems", meta = (WorldContext = "WorldContextObject"))
// 	static UWorldManagerSubsystem* GetUISubsystem(const UObject* WorldContextObject);


	// ----------------------------------------------------------------------
	// 3. GAS 및 컴포넌트 접근 헬퍼 (GAS & Component Accessors)
	// ----------------------------------------------------------------------

	/** * 액터로부터 UAEAbilitySystemComponent를 가져옵니다.
	 * IAbilitySystemInterface를 확인하고, 없으면 컴포넌트를 직접 찾습니다.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|AbilitySystem")
	static UAEAbilitySystemComponent* GetAbilitySystemComponent(const AActor* Actor);

	/** * 액터로부터 기본 UAS_BaseCombat을 가져옵니다. 
	 * (ASC가 유효해야 합니다)
	 * @note 반환이 const입니다.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|AbilitySystem")
	static const UAS_BaseCombat* GetBaseAttributeSet(const AActor* Actor);

	/** * 액터로부터 기본 UAS_Health을 가져옵니다. 
	 * (ASC가 유효해야 합니다)
	 * @note 반환이 const입니다.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|AbilitySystem")
	static const UAS_HealthSet* GetHealthAttributeSet(const AActor* Actor);

	/** 액터로부터 특정 타입의 컴포넌트를 안전하게 가져옵니다. (템플릿) */
	template<typename T>
	static T* GetActorComponent(const AActor* Actor)
	{
		if (Actor)
		{
			return Actor->FindComponentByClass<T>();
		}
		return nullptr;
	}


	// ----------------------------------------------------------------------
	// 4. 게임플레이 로직 헬퍼 (Gameplay Logic Helpers)
	// ----------------------------------------------------------------------

	/** 대상 액터가 '살아있는지' 확인합니다. (ASC 및 Health 어트리뷰트 확인) */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Combat")
	static bool IsAlive(const AActor* Actor);

	/** 대상 액터의 현재 체력을 가져옵니다. (없으면 0.f) */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Combat")
	static float GetHealth(const AActor* Actor);

	/** 대상 액터의 최대 체력을 가져옵니다. (없으면 0.f) */
	UFUNCTION(BlueprintPure, Category = "ProjectAE|Combat")
	static float GetMaxHealth(const AActor* Actor);

	/** * 대상(Target)에게 데미지 GameplayEffect를 적용하는 편의 함수입니다.
	 * @param Instigator 데미지를 유발한 액터 (컨트롤러)
	 * @param Causer 데미지를 직접 발생시킨 액터 (무기, 발사체)
	 * @param Target 데미지를 입는 액터
	 * @param DamageEffect TSubclassOf<UGameplayEffect> 데미지 계산에 사용할 GE
	 * @param DamageAmount 데미지 양 (GE에서 SetByCaller로 사용)
	 * @note 추후 구현합니다.
	 */
// 	UFUNCTION(BlueprintCallable, Category = "ProjectAE|Combat")
// 	static void ApplyDamage(AActor* Instigator, AActor* Causer, AActor* Target, TSubclassOf<class UGameplayEffect> DamageEffect, float DamageAmount);

	/** * 퀘스트 시스템에 이벤트를 전송합니다. (예: 적 처치, 아이템 획득)
	 * (내부적으로 GetQuestSubsystem()->HandleEvent(...)를 호출합니다)
	 */
// 	UFUNCTION(BlueprintCallable, Category = "ProjectAE|Quest", meta = (WorldContext = "WorldContextObject"))
// 	static void SendQuestEvent(const UObject* WorldContextObject, AActor* Instigator, FGameplayTag EventTag, FName TargetID, int32 Amount = 1);


	// ----------------------------------------------------------------------
	// 5. 유틸리티 및 디버그 헬퍼 (Utility & Debug)
	// ----------------------------------------------------------------------

	/** (디버그용) 화면에 메시지를 출력합니다. (GEngine 사용) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAE|Debug")
	static void PrintString(const FString& Message, float Duration = 5.0f, FColor Color = FColor::White, bool bPrintToLog = false);
};
