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
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/HealthComponent.h"
#include "Characters/Player/OcclusionFadeComponent.h"
#include "Components/WidgetComponent.h"
#include "Widgets/StatBarWidget.h"
#include "GameFramework/CharacterMovementComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	StaminaWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("StaminaWidgetComponent");
	StaminaWidgetComponent->SetupAttachment(RootComponent);

	WeaponComponent = CreateDefaultSubobject<UAEWeaponComponent>("WeaponComponent");

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");

	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
	InventoryComponent->bIsPlayerInventory = true;
	
	OcclusionFadeComponent = CreateDefaultSubobject<UOcclusionFadeComponent>("OcclusionFadeComponent");

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

	// InteractionComponent의 포커스 변경 시 호출될 콜백 함수 등록
	InteractionComponent->OnFocusChanged.AddDynamic(this, &APlayerCharacter::OnFocusChanged);

	TargetRotation = GetActorRotation();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeathDelegate.AddDynamic(this, &APlayerCharacter::OnDeath);
	HealthComponent->OnDamageDelegate.AddDynamic(this, &APlayerCharacter::OnDamaged);
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
	for (const TSubclassOf<UGameplayAbility>& AbilityForGrant : DefaultAbilities)
	{
		FGameplayAbilitySpec Spec(AbilityForGrant, 1, -1, this);
		CachedASC->GiveAbility(Spec);
	}

	if (CachedASC)
	{
		// 헬스컴포넌트 초기화 시도
		HealthComponent->TryInitAbilitySystem(CachedASC);

		// 기본 무기 장착 및 무기의 어빌리티 부여
		WeaponComponent->EquipWeapon(DefaultWeapon);

		if (StaminaWidgetComponent)
		{
			StaminaWidgetComponent->InitWidget();

			// 스테미나 위젯 바인드
			UStatBarWidgetBase* StaminaWidget = Cast<UStatBarWidgetBase>(StaminaWidgetComponent->GetUserWidgetObject());
			if (StaminaWidget)
			{
				StaminaWidget->BindToASC(CachedASC);
			}
		}
	}
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

void APlayerCharacter::OnDeath_Implementation(AActor* Causer, AActor* Victim)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// 입력 차단 (움직임, 공격 등 금지)
		DisableInput(PC);
	}

	if (UCharacterMovementComponent* CharMove = GetCharacterMovement())
	{
		// 이동을 완전히 비활성화
		CharMove->StopMovementImmediately();
		CharMove->DisableMovement();
	}

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		// 메쉬를 루트(캡슐)로부터 분리하여 월드에 독립시킵니다.
		SkelMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	// 2) 캡슐 충돌 비활성화 (ragdoll과 충돌 방지)
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	// 3) 메쉬를 ragdoll로 전환하여 자연스럽게 쓰러지게 함
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		// 충돌 프로필을 ragdoll로 설정(프로젝트에 프로필이 없으면 QueryAndPhysics 사용)
		SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));

		// 물리 시뮬레이션 시작
		SkelMesh->SetAllBodiesSimulatePhysics(true);
		SkelMesh->SetSimulatePhysics(true);
		SkelMesh->WakeAllRigidBodies();
		SkelMesh->bBlendPhysics = true; // 애니와 물리 블렌드 허용(자연스러운 전환)

		// 현재 물리 속도가 너무 크면 클램프하여 날아가는 현상 방지
		const float MaxLinearSpeed = 600.0f; // 필요 시 튜닝
		FVector CurrentLinear = SkelMesh->GetPhysicsLinearVelocity(NAME_None);
		if (CurrentLinear.Size() > MaxLinearSpeed)
		{
			SkelMesh->SetAllPhysicsLinearVelocity(CurrentLinear.GetClampedToMaxSize(MaxLinearSpeed));
		}

		// 각속도 제거하여 불필요한 회전 최소화
		SkelMesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		// 감쇄 설정으로 빠르게 안정화
		SkelMesh->SetLinearDamping(4.0f);
		SkelMesh->SetAngularDamping(4.0f);

		// 약한 임펄스를 주면 더 자연스럽게 넘어짐 (Causer가 있으면 그 반대 방향으로 가볍게)
		if (Causer)
		{
			FVector ImpulseDir = (SkelMesh->GetComponentLocation() - Causer->GetActorLocation()).GetSafeNormal();
			const float ImpulseStrength = 250.0f; // 필요 시 튜닝
			SkelMesh->AddImpulse(ImpulseDir * ImpulseStrength, NAME_None, true);
		}
	}
}

void APlayerCharacter::OnDamaged_Implementation(AActor* Causer, AActor* Victim)
{
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