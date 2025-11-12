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

}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 어빌리티 시스템 초기화
	InitAbiltySystem();

	// Enemy 생성 시 Crash 발생
	if (!GetPlayerState()) return;
	
	CachedASC->InitAbilityActorInfo(GetPlayerState(), this);
	// TODO: 어빌리티 부여 등 추가 초기화 작업
}

TWeakObjectPtr<UAbilitySystemComponent> ABaseCharacter::GetASC() const
{
	return CachedASC;
}

void ABaseCharacter::InitAbiltySystem()
{
	APlayerState* PS = GetPlayerState();
	if (ensure(PS))
	{
		UAbilitySystemComponent* FoundASC = Cast<AAEPlayerState>(PS)->GetAbilitySystemComponent();
		if (ensure(FoundASC))
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