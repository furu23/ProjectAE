// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilityInputConfig;

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
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<class UCameraComponent> Camera;


	// **** 어빌리티 시스템 인풋 바인딩 ****

	// 인풋 액션이 눌렸을 때 호출되는 함수
	void InputAbilityTagPressed(const class UInputAction* Action);
	// 인풋 액션이 떼졌을 때 호출되는 함수
	void InputAbilityTagReleased(const class UInputAction* Action);

	UPROPERTY(EditDefaultsOnly, Category = "Input|Ability", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityInputConfig> AbilityInputConfig;
	
};
