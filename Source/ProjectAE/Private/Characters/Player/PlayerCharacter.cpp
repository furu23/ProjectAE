// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/PlayerCharacter.h"

#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "Data/AbilityInputConfig.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Interaction/InteractionComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");

	SpringArm->TargetArmLength = 1000.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;

	// InteractionComponent의 포커스 변경 시 호출될 콜백 함수 등록
	InteractionComponent->OnFocusChanged.AddDynamic(this, &APlayerCharacter::OnFocusChanged);
	
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
			if (ensure(InputAction))
			{
				// 눌림 이벤트 바인딩
				EnhancedInput->BindAction(InputAction, ETriggerEvent::Started, this, &APlayerCharacter::InputAbilityTagPressed, InputAction);
				// 떼짐 이벤트 바인딩
				EnhancedInput->BindAction(InputAction, ETriggerEvent::Completed, this, &APlayerCharacter::InputAbilityTagReleased, InputAction);
			}
		}
	}

}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UAbilitySystemComponent* ASC = CachedASC.Get();
	for (TSubclassOf<UGameplayAbility> AbilityForGrant : DefaultAbilities)
	{
		if (AbilityForGrant)
		{
			const FGameplayAbilitySpec Spec(AbilityForGrant, 1, -1, this);
			ASC->GiveAbility(Spec);
		}
	}
}

void APlayerCharacter::OnFocusChanged(AActor* NewFocusedActor)
{
	if (NewFocusedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerChar : NewFocusedActor %s"), *NewFocusedActor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APlayerChar : NewFocusedActor NULL"));
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
	if (ensure(FoundTag && FoundTag->IsValid()))
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
