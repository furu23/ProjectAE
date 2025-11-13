// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AEGloabalHelper.h"

// --- 필요한 엔진 헤더 ---
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h" // GetPlayerController 등
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Engine/Engine.h" // GEngine (디버그 프린트용)

// --- 프로젝트 커스텀 헤더 ---
// (실제 프로젝트의 헤더 경로에 맞게 수정해야 합니다)
#include "Core/AEGameMode.h"
#include "Characters/Player/AEPlayerController.h"
// #include "AEPlayerState.h"
#include "Characters/Player/PlayerCharacter.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/AS_BaseCombat.h"
#include "AbilitySystem/AS_HealthSet.h"
#include "Characters/AEPlayerState.h"
#include "Quest/QuestManagerSubSystem.h"


// ----------------------------------------------------------------------
// 1. 프레임워크 접근 헬퍼 (Framework Accessors)
// ----------------------------------------------------------------------

AAEGameMode* UAEGloabalHelper::GetAEGameMode(const UObject* WorldContextObject)
{
	// UGameplayStatics::GetGameMode를 사용하고 커스텀 클래스로 캐스팅합니다.
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		// GetGameMode는 서버에서만 유효합니다.
		return Cast<AAEGameMode>(World->GetAuthGameMode());
	}
	return nullptr;
}

/*
AAEGameState* UAEGloabalHelper::GetMyGameState(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return Cast<AAEGameState>(World->GetGameState());
	}
	return nullptr;
}
*/

AAEPlayerController* UAEGloabalHelper::GetAEPlayerController(const UObject* WorldContextObject)
{
	// 로컬 플레이어(Index 0)의 컨트롤러를 가져옵니다.
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return Cast<AAEPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
	}
	return nullptr;
}

AAEPlayerState* UAEGloabalHelper::GetAEPlayerState(const UObject* WorldContextObject)
{
	if (AAEPlayerController* PC = GetAEPlayerController(WorldContextObject))
	{
		return PC->GetPlayerState<AAEPlayerState>();
	}
	return nullptr;
}


APlayerCharacter* UAEGloabalHelper::GetAECharacter(const UObject* WorldContextObject)
{
	if (AAEPlayerController* PC = GetAEPlayerController(WorldContextObject))
	{
		return Cast<APlayerCharacter>(PC->GetPawn());
	}
	return nullptr;
}

AAEPlayerController* UAEGloabalHelper::GetOwningPlayerController(UObject* TargetObject)
{
	if (!TargetObject)
	{
		return nullptr;
	}

	// 1. 액터 자체가 컨트롤러일 수 있습니다.
	if (AAEPlayerController* PC = Cast<AAEPlayerController>(TargetObject))
	{
		return PC;
	}

	// 2. 폰(캐릭터)의 컨트롤러를 찾습니다.
	if (const APawn* Pawn = Cast<APawn>(TargetObject))
	{
		return Cast<AAEPlayerController>(Pawn->GetController());
	}

	// 3. 컴포넌트일 경우, 소유자(Owner)를 찾습니다.
	if (const UActorComponent* Comp = Cast<UActorComponent>(TargetObject))
	{
		return GetOwningPlayerController(Comp->GetOwner());
	}

	// 4. 일반 액터(예: 무기)일 경우, 소유자를 찾습니다.
	if (AActor* Actor = Cast<AActor>(TargetObject))
	{
		if (AAEPlayerController* PC = Cast<AAEPlayerController>(GetOwningPlayerController(Actor->GetOwner())))
		{
			return PC;
		}
	}
	return nullptr;
}


// ----------------------------------------------------------------------
// 2. 서브시스템 접근 헬퍼 (Subsystem Accessors)
// ----------------------------------------------------------------------


UQuestManagerSubSystem* UAEGloabalHelper::GetQuestSubsystem(const UObject* WorldContextObject)
{
	const APlayerController* PC = GetAEPlayerController(WorldContextObject);
	if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<UQuestManagerSubSystem>();
	}
	return nullptr;
}


// ----------------------------------------------------------------------
// 3. GAS 및 컴포넌트 접근 헬퍼 (GAS & Component Accessors)
// ----------------------------------------------------------------------

UAEAbilitySystemComponent* UAEGloabalHelper::GetAbilitySystemComponent(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// 1. IAbilitySystemInterface를 우선 확인합니다. (가장 표준적인 방법)
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
	{
		// 커스텀 ASC 클래스로 캐스팅 시도
		if (UAEAbilitySystemComponent* ASC = Cast<UAEAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
		{
			return ASC;
		}
	}

	// 2. 인터페이스가 없다면, 액터가 직접 컴포넌트를 가지고 있는지 찾습니다.
	// (예: 무기나 환경 오브젝트에 ASC가 붙어있는 경우)
	 if (UAEAbilitySystemComponent * ASC = Actor->FindComponentByClass<UAEAbilitySystemComponent>())
	 {
		return ASC;
	 }

	 // 3. PlayerState에 접근할 수 있는지 확인합니다.
	 // (예: Pawn, Character)
	 // UGlobalHelper::GetPlayerState를 이용하지 않습니다.
	 if (const APawn* Pawn = Cast<APawn>(Actor))
	 {
		 APlayerState* PS = Pawn->GetPlayerState();
		 if (PS)
		 {
			 if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
			 {
				 // 커스텀 ASC 클래스로 캐스팅 시도
				 if (UAEAbilitySystemComponent* ASC = Cast<UAEAbilitySystemComponent>(ASI->GetAbilitySystemComponent()))
				 {
					 return ASC;
				 }
			 }
		 }
	 }
	 return nullptr;
}

const UAS_BaseCombat* UAEGloabalHelper::GetBaseAttributeSet(const AActor* Actor)
{
	// ASC 헬퍼를 재사용합니다.
	if (UAEAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor))
	{
		// ASC가 AttributeSet을 소유하고 있으므로, ASC에서 가져옵니다.
		// (GetSet<T>는 UAbilitySystemComponent에 내장된 템플릿 함수)
		return ASC->GetSet<UAS_BaseCombat>();
	}
	return nullptr;
}

const UAS_HealthSet* UAEGloabalHelper::GetHealthAttributeSet(const AActor* Actor)
{
	// ASC 헬퍼를 재사용합니다.
	if (UAEAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor))
	{
		// ASC가 AttributeSet을 소유하고 있으므로, ASC에서 가져옵니다.
		// (GetSet<T>는 UAbilitySystemComponent에 내장된 템플릿 함수)
		return ASC->GetSet<UAS_HealthSet>();
	}
	return nullptr;
}


// ----------------------------------------------------------------------
// 4. 게임플레이 로직 헬퍼 (Gameplay Logic Helpers)
// ----------------------------------------------------------------------

bool UAEGloabalHelper::IsAlive(const AActor* Actor)
{
	// GetHealth 헬퍼를 사용합니다.
	// (0.0f 초과인지 확인)
	return GetHealth(Actor) > 0.0f;
}

float UAEGloabalHelper::GetHealth(const AActor* Actor)
{
	if (const UAS_HealthSet* AttributeSet = GetHealthAttributeSet(Actor))
	{
		// AttributeSet의 GetHealth() 함수를 호출합니다.
		// (FGameplayAttributeData의 GetCurrentValue()를 직접 호출해도 됩니다)
		return AttributeSet->GetHealth();
	}
	// ASC나 AttributeSet이 없으면 0.0f를 반환합니다.
	return 0.0f;
}

float UAEGloabalHelper::GetMaxHealth(const AActor* Actor)
{
	if (const UAS_HealthSet* AttributeSet = GetHealthAttributeSet(Actor))
	{
		return AttributeSet->GetMaxHealth();
	}
	return 0.0f;
}

/*
void UAEGloabalHelper::ApplyDamage(AActor* Instigator, AActor* Causer, AActor* Target, TSubclassOf<UGameplayEffect> DamageEffect, float DamageAmount)
{
	if (!DamageEffect || !Target || DamageAmount <= 0.0f)
	{
		return;
	}

	UMyAbilitySystemComponent* TargetASC = GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		// ASC가 없는 대상에게는 데미지를 줄 수 없습니다. (GAS 기반이므로)
		return;
	}

	// 1. GameplayEffectContext 생성
	// Instigator(가해자 컨트롤러)와 Causer(실제 데미지 원인, 예: 무기)를 설정합니다.
	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	ContextHandle.AddInstigator(Instigator, Causer);

	// 2. GameplayEffectSpec 생성
	// EffectSpec은 이펙트가 실제로 적용되기 전의 '설계도'입니다.
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	// 3. SetByCaller를 사용하여 데미지 양을 GE로 전달
	// (Damage GE는 "SetByCaller"로 "Damage"라는 태그의 값을 받도록 설정되어 있어야 함)
	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Damage")), DamageAmount);

	// 4. 이펙트 적용
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}*/

/*
void UAEGloabalHelper::SendQuestEvent(const UObject* WorldContextObject, AActor* Instigator, FGameplayTag EventTag, FName TargetID, int32 Amount)
{
	// 퀘스트 서브시스템 헬퍼를 사용합니다.
	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		// 퀘스트 서브시스템의 내부 함수로 이벤트를 전달합니다.
		// (AMyPlayerController* InstigatorPC = GetOwningPlayerController(Instigator); 를 찾아 전달하는 것이 더 정확할 수 있습니다)
		QuestSubsystem->HandleQuestEvent(Instigator, EventTag, TargetID, Amount);
	}
}*/


// ----------------------------------------------------------------------
// 5. 유틸리티 및 디버그 헬퍼 (Utility & Debug)
// ----------------------------------------------------------------------

void UAEGloabalHelper::PrintString(const FString& Message, float Duration, FColor Color, bool bPrintToLog)
{
	// GEngine이 유효한지 확인 (에디터/게임에서만 작동)
	if (GEngine)
	{
		// 화면에 디버그 메시지 출력
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}

	if (bPrintToLog)
	{
		// 로그 파일에 출력 (LogTemp 카테고리 사용)
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}
}
