// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "BaseInteractableActor.generated.h"


UCLASS()
class PROJECTAE_API ABaseInteractableActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnInteract_Implementation(AActor* Caller) override;
	virtual void OnFocusChanged_Implementation(AActor* Caller, bool bIsFocused) override;
	
	virtual bool CanInteract_Implementation() const override;
	virtual bool CanFocus_Implementation() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool bCanBeFocused = true;

	/** 
	 *  Warning: Interaction will not occur if `bCanBeFocused` is `false`
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool bAllowInteraction = true;

};
