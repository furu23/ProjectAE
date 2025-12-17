// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "AbilitySystem/Data/AbilityInputConfig.h"
#include "Characters/AEPlayerState.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputComponent.h"
#include "Core/AEGlobalHelper.h"
#include "FX/AEAudioComponent.h"
#include "Components/WidgetComponent.h"
#include "Widgets/StatBarWidget.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 스텟 바 생성 및 부착
	StatBarComp = CreateDefaultSubobject<UWidgetComponent>("StatBarComp");
	StatBarComp->SetupAttachment(RootComponent);

	// 기타 초기 설정
	StatBarComp->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	StatBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	StatBarComp->SetDrawAtDesiredSize(true);
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
}

UAbilitySystemComponent* ABaseCharacter::GetASC() const
{
	return CachedASC;
}

// ======= Development Build Only =======

void ABaseCharacter::AddLooseTagForDevelop(FGameplayTag Tag, const UObject* UserClass, bool bIsForStacking)
{
	if (CachedASC)
	{
		if (!bIsForStacking)
		{
			if (CachedASC->GetGameplayTagCount(Tag) < 1)
			{
				CachedASC->AddLooseGameplayTag(Tag);
			}
		}
		else
		{ 
			CachedASC->AddLooseGameplayTag(Tag);
		}
		UE_LOG(LogTemp, Log, TEXT("Tag `%s` is Added in Object `%s`"), *Tag.GetTagName().ToString(), *UserClass->GetFName().ToString());
	}
}

void ABaseCharacter::RemoveLooseTagForDevelop(FGameplayTag Tag, const UObject* UserClass, bool bIsRemoveAll)
{
	if (CachedASC)
	{
		if (bIsRemoveAll)
		{
			CachedASC->RemoveLooseGameplayTag(Tag, CachedASC->GetGameplayTagCount(Tag));
		}
		else
		{
			CachedASC->RemoveLooseGameplayTag(Tag);
		}
		UE_LOG(LogTemp, Log, TEXT("Tag `%s` is Removed in Object `%s`"), *Tag.GetTagName().ToString(), *UserClass->GetFName().ToString());
	}	
}

// ======================================

void ABaseCharacter::InitAbiltySystem()
{
	UAbilitySystemComponent* FoundASC = UAEGlobalHelper::GetAbilitySystemComponent(this);
	if (ensure(FoundASC))
	{
		CachedASC = FoundASC;
		CachedASC->InitAbilityActorInfo(FoundASC->GetOwner(), this);
	}

	if (CachedASC && StatBarComp)
	{
		StatBarComp->InitWidget();

		ULinearStatBarWidget* HealthStatBar = Cast<ULinearStatBarWidget>(StatBarComp->GetUserWidgetObject());

		if (HealthStatBar)
		{
			HealthStatBar->BindToASC(CachedASC);
		}
	}
}

/*
void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbiltySystem();
}*/