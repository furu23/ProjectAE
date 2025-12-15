// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilityInputConfig;
class UGameplayAbility;
struct FInputAbilitySet;
class UAEWeaponDefinition;

/**
 * 
 */
UCLASS()
class PROJECTAE_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void OnFocusChanged(AActor* NewFocusedActor);

protected:
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Move(const FVector2D& MoveVector);
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RotateToCursor();

	// 캐릭터가 죽었을 때 호출될 함수
	UFUNCTION(BlueprintNativeEvent, Category = "Character|Event", meta = (ToolTip = "사망 시에 GMS 이전 시전에 호출됩니다."))
	void OnDeath(AActor* Causer, AActor* Victim);

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Event", meta = (ToolTip = "데미지를 받았을 때 호출됩니다. 사망 시점에서는 호출되지 않습니다."))
	void OnDamaged(AActor* Causer, AActor* Victim);
	
private:
	FRotator TargetRotation;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RotationInterpSpeed = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UInteractionComponent> InteractionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UInventoryComponent> InventoryComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UOcclusionFadeComponent> OcclusionFadeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UAEWeaponComponent> WeaponComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class AAEPlayerController> AEPlayerController;


	// **** 어빌리티 ****

	UPROPERTY(BlueprintReadWrite, Category = "Ability", meta = (ToolTip = "조준 중인지에 관련된 bool 프로퍼티"))
	bool bIsAiming = false;

	// **** 어빌리티 시스템 인풋 바인딩 ****

	// 인풋 액션이 눌렸을 때 호출되는 함수
	void InputAbilityTagPressed(const class UInputAction* Action);
	// 인풋 액션이 떼졌을 때 호출되는 함수
	void InputAbilityTagReleased(const class UInputAction* Action);

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityInputConfig> AbilityInputConfig;

	// 기본 어빌리티 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// 기본 무기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category ="Ability")
	TObjectPtr<UAEWeaponDefinition> DefaultWeapon;

public:
	FORCEINLINE class UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
};
