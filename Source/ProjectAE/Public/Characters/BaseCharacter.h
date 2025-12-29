// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "BaseCharacter.generated.h"

class UAbilitySystemComponent;
class UHitFeedback;
class UAEAudioComponent;
class UWidgetComponent;
class UGameplayEffect;

UCLASS()
class PROJECTAE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;



	// 태그 반환용 게터 함수
	FORCEINLINE const FGameplayTagContainer& GetCharacterTag() const { return CharacterTag; }

	// ASC를 반환하는 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Ability")
	UAbilitySystemComponent* GetASC() const;



	// **** Develop Test&Debug Only ****

	/**
	* @brief 편하게 태그를 추가해주는 헬퍼 함수입니다.
	* 
	* @param TagRef			추가할 태그
	* @param UserClass		이 함수를 호출하는 클래스 (ex: AddLooseTagForDevelop(Tag , this); )
	* @param bIsForStacking	한 번만 적용할지 확인합니다.
	* 
	* @note 이 함수는 Development Build 에서만 작동하며, Shipping Build에서는 아무런 작동도 하지 않습니다.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability",
		meta=(DevelopmentOnly, DisplayName = "AddTag", ToolTip = "개발 빌드용 태그 추가 헬퍼 함수입니다.")
	)
	virtual void AddLooseTagForDevelop(FGameplayTag Tag, const UObject* UserClass, bool bIsForStacking = true);

	/**
	* @brief 편하게 태그를 제거해주는 헬퍼 함수입니다.
	*
	* @param TagRef			제거할 태그
	* @param UserClass		이 함수를 호출하는 클래스 (ex: RemoveLooseTagForDevelop(Tag , this); )
	* @param bIsRemoveAll	모든 스택 태그를 지울 지 확인합니다.
	*
	* @note 이 함수는 Development Build 에서만 작동하며, Shipping Build에서는 아무런 작동도 하지 않습니다.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability",
		meta=(DevelopmentOnly, DisplayName = "RemoveTag", ToolTip = "개발 빌드용 태그 제거 헬퍼 함수입니다.")
	)
	virtual void RemoveLooseTagForDevelop(FGameplayTag Tag, const UObject* UserClass, bool bIsRemoveAll = true);

	// *********************************

protected:
	// *** 어빌리티 시스템 관련 기능 ***

	// 어빌리티 시스템 초기화
	void InitAbiltySystem();

	// 클라이언트에서 플레이어 상태가 복제될 때 호출되는 함수 (어빌리티 시스템 초기화에 사용)
	// void OnRep_PlayerState() override;

	// 어빌리티 시스템 컴포넌트 캐시
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;


	// **** VFX / SFX 관련 ****

	// 풋스텝 VFX DA
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|FX")
	TObjectPtr<UHitFeedback> FootstepMap;


	// **** 기타 컴포넌트 ****

	// 체력바용 위젯 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|UI")
	TObjectPtr<UWidgetComponent> StatBarComp;


	// **** 기타 ****

	// 캐릭터의 태그 목록 (예: Gunner, Enemy 등...)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true", ToolTip = "캐릭터의 태그 목록입니다. GMS 발송 시 사용."))
	FGameplayTagContainer CharacterTag;
};