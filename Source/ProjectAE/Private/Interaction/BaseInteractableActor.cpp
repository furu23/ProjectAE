// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/BaseInteractableActor.h"


// Sets default values
ABaseInteractableActor::ABaseInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseInteractableActor::OnInteract_Implementation(AActor* Caller)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Interacting with %s"), *Caller->GetName()));
}

void ABaseInteractableActor::OnFocusChanged_Implementation(AActor* Caller, bool bIsFocused)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Focus changed %s to %s"), bCanBeFocused ? TEXT("true") : TEXT("false"), *Caller->GetName()));
}

bool ABaseInteractableActor::CanInteract_Implementation() const
{
	return bAllowInteraction;
}

bool ABaseInteractableActor::CanFocus_Implementation() const
{
	return bCanBeFocused;
}
