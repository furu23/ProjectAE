// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Data/AbilityInputConfig.h"
#include "Characters/AEPlayerState.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);

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
				EnhancedInput->BindAction(InputAction, ETriggerEvent::Started, this, &ABaseCharacter::InputAbilityTagPressed, InputAction);
				// 떼짐 이벤트 바인딩
				EnhancedInput->BindAction(InputAction, ETriggerEvent::Completed, this, &ABaseCharacter::InputAbilityTagReleased, InputAction);
			}
		}
	}

}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 어빌리티 시스템 초기화
	InitAbiltySystem();

	// TODO: 어빌리티 부여 등 추가 초기화 작업
}

void ABaseCharacter::InitAbiltySystem()
{
	APlayerState* PS = GetPlayerState();
	if (PS)
	{
		UAbilitySystemComponent* FoundASC = Cast<AAEPlayerState>(PS)->GetAbilitySystemComponent();
		if (FoundASC)
		{
			CachedASC = FoundASC;
		}
	}
}

void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbiltySystem();
}

void ABaseCharacter::InputAbilityTagPressed(const class UInputAction* Action)
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

void ABaseCharacter::InputAbilityTagReleased(const class UInputAction* Action)
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

