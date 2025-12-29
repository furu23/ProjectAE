// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SaveGameSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVolumType : uint8
{
	Master = 0,
	BGM,
	SFX,
	Voice
};


/**
 * @brief 저장을 행하는 서브시스템입니다.
 * @note UPROPERTY의 SaveGame 지정자를 이용하도록 만들어졌습니다.
 * 
 * SaveGame 지정자를 사용하지 않는다면 말해주세요.
 * 오버로딩 함수를 추가하는 것도 방법입니다.
 */
UCLASS()
class PROJECTAE_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// **** 인게임 값 저장/로드 ****

	// 게임 새 시작시 필요할 경우
	UFUNCTION(BlueprintCallable)
	void NewGame();

	// 세이브 게임 용도
	UFUNCTION(BlueprintCallable)
	void SaveGame(bool bSetAsyncLoad = false);

	// 로드 게임 용도
	UFUNCTION(BlueprintCallable)
	void LoadGame();


	// **** 설정값 저장/로드 ****

	void LoadGlobalSettings();
    
    UFUNCTION(BlueprintCallable)
    void SaveGlobalSettings();

	// 볼륨 조절 헬퍼 함수
    UFUNCTION(BlueprintCallable)
    void SetVolume(EVolumType Type, float NewVolume);

    UFUNCTION(BlueprintCallable)
    float GetVolume(EVolumType Type) const;


	// **** 인벤토리 관련 ****

	// 인벤토리 캐시 저장
	void SaveInventoryToCache(const TArray<uint8>& Data);

	// 인벤토리 캐시 로드
	bool GetInventoryFromCache(TArray<uint8>& OutData);


	// **** 전역 이벤트 관련 ****
	
	// 태그가 이미 완료되었는지 확인
	UFUNCTION(BlueprintCallable)
	bool IsAllEventCompleted(const FGameplayTagContainer& EventTag) const;

	UFUNCTION(BlueprintCallable)
	bool IsEventCompleted(const FGameplayTag& EventTag) const;

	// 사건 완료 처리
	UFUNCTION(BlueprintCallable)
	void MarkAllEventCompleted(const FGameplayTagContainer& EventTag);

	UFUNCTION(BlueprintCallable)
	void MarkEventCompleted(const FGameplayTag& EventTag);

private:
	UPROPERTY()
	TArray<uint8> PlayerInventoryCache;

	UPROPERTY()
	TArray<float> CachedVolumeData;

	// 로드해둔 세이브 전역 이벤트
	FGameplayTagContainer LoadedCompletedEvents;

	// 설정 저장 파일 이름
	const FString GlobalOptionSlotName = TEXT("GlobalOptionSave");
};
