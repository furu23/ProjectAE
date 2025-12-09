// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/BaseEnemyCharacter.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/AS_HealthSet.h"
#include "AbilitySystem/HealthComponent.h"
#include "Components/CapsuleComponent.h"


UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	if (ASC)
	{
		return ASC;
	}
	return nullptr;
}

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	ASC = CreateDefaultSubobject<UAEAbilitySystemComponent>("ASC");

	HealthComp = CreateDefaultSubobject<UHealthComponent>("HealthComp");
	HealthSet = CreateDefaultSubobject<UAS_HealthSet>("HealthSet");

	// AttributeSet 등록
	ASC->AddAttributeSetSubobject<UAS_HealthSet>(HealthSet);
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnDeathDelegate.AddDynamic(this, &ABaseEnemyCharacter::OnDeath);
		HealthComp->OnDamageDelegate.AddDynamic(this, &ABaseEnemyCharacter::OnDamaged);
	}
}

void ABaseEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HealthComp)
	{
		HealthComp->TryInitAbilitySystem(ASC);
	}
}

void ABaseEnemyCharacter::OnDeath_Implementation(AActor* Causer, AActor* Victim)
{
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseEnemyCharacter::OnDamaged_Implementation(AActor* Causer, AActor* Victim)
{

}
