// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/PlayerCharacter.h"

#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "Data/AbilityInputConfig.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	SpringArm->TargetArmLength = 1000.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	/*
	* 기타 로직
	*/

	// **** 어빌리티 시스템 인풋 바인딩 ****
	if (EnhancedInput && AbilityInputConfig)
	{
		for (const TPair<TObjectPtr<UInputAction>, FGameplayTag>& Pair : AbilityInputConfig->AbilityInputActions)
		{
			const UInputAction* InputAction = Pair.Key;
			const FGameplayTag BindingTag = Pair.Value;
			if (InputAction)
			{
				// 눌림 이벤트 바인딩
				EnhancedInput->BindAction(InputAction, ETriggerEvent::Started, this, &APlayerCharacter::InputAbilityTagPressed, InputAction);
				// 떼짐 이벤트 바인딩
				EnhancedInput->BindAction(InputAction, ETriggerEvent::Completed, this, &APlayerCharacter::InputAbilityTagReleased, InputAction);
			}
		}
	}

}

void APlayerCharacter::InputAbilityTagPressed(const class UInputAction* Action)
{
	UAEAbilitySystemComponent* ASC = Cast<UAEAbilitySystemComponent>(CachedASC.Get());
	if (!Action || !ASC)
	{
		return;
	}

	const FGameplayTag* FoundTag = AbilityInputConfig->AbilityInputActions.Find(Action);
	if (FoundTag && FoundTag->IsValid())
	{
		ASC->AbilityInputTagPressed(*FoundTag);
	}
}

void APlayerCharacter::InputAbilityTagReleased(const class UInputAction* Action)
{
	UAEAbilitySystemComponent* ASC = Cast<UAEAbilitySystemComponent>(CachedASC.Get());
	if (!Action || !ASC)
	{
		return;
	}
	const FGameplayTag* FoundTag = AbilityInputConfig->AbilityInputActions.Find(Action);
	if (FoundTag && FoundTag->IsValid())
	{
		ASC->AbilityInputTagReleased(*FoundTag);
	}

}
