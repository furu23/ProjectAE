// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/PlayerCharacter.h"

#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInputConfig.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Characters/Player/AEPlayerController.h"
#include "Interaction/InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "GameplayTagContainer.h"
#include "Characters/Player/AEWeaponComponent.h"
#include "../ProjectAE.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	WeaponComponent = CreateDefaultSubobject<UAEWeaponComponent>("WeaponComponent");

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
	InventoryComponent->bIsPlayerInventory = true;

	SpringArm->TargetArmLength = 1000.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bDoCollisionTest = false;
	
	SpringArm->bEnableCameraLag = true;

	// 캐릭터랑 같이 회전 안함 (TopDown 에서는 고정 방식)
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// GetCharacterMovement()->bOrientRotationToMovement = false;

	// InteractionComponent의 포커스 변경 시 호출될 콜백 함수 등록
	InteractionComponent->OnFocusChanged.AddDynamic(this, &APlayerCharacter::OnFocusChanged);

	TargetRotation = GetActorRotation();
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

	AAEPlayerController* PC = Cast<AAEPlayerController>(NewController);
	if (PC)
	{
		AEPlayerController = PC;
	}
	
	// 어빌리티 초기화
	UAbilitySystemComponent* ASC = CachedASC.Get();
	for (const TSubclassOf<UGameplayAbility>& AbilityForGrant : DefaultAbilities)
	{
		FGameplayAbilitySpec Spec(AbilityForGrant, 1, -1, this);
		ASC->GiveAbility(Spec);
	}

	// 기본 무기 장착 및 무기의 어빌리티 부여
	WeaponComponent->EquipWeapon(DefaultWeapon);

	// TODO: 변경 시 전파받아 상태 부여
	#if UE_BUILD_DEVELOPMENT
	ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Area.InRaid"));
	#endif
}

void APlayerCharacter::OnFocusChanged(AActor* NewFocusedActor)
{
	if (!AEPlayerController) return;
	
	AEPlayerController->OnInteractionFocusChanged(NewFocusedActor);
}

void APlayerCharacter::Move(const FVector2D& MoveVector)
{
	if (!GetController()) return;

	const FRotator NewRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, NewRotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveVector.Y);
	AddMovementInput(RightDirection, MoveVector.X);
}

void APlayerCharacter::RotateToCursor()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	
	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		FVector Direction = Hit.ImpactPoint - GetActorLocation();
		Direction.Z = 0.f;

		TargetRotation = Direction.Rotation();
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