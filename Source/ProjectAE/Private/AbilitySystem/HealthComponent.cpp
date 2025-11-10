// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/HealthComponent.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UAbilitySystemComponent* UHealthComponent::TryFindAbiltySystemComponent() const
{
	return nullptr;
}
