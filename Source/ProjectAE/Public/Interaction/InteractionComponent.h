// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, AActor*, NewFocusedActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FTimerHandle TraceTimerHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void TryInteract();
	
	void BeginTracing();
	void StopTracing();

private:
	void UpdateTracing();

	AActor* PerformTrace(const FVector& TraceStart, const FVector& TraceEnd);

	bool GetTracePoint(FVector& OutStart, FVector& OutEnd);

public:
	UPROPERTY(BlueprintAssignable, Category = "Interact")
	FOnFocusChanged OnFocusChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float TraceInterval = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float TraceDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
	AActor* CurrentFocus = nullptr;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
	AActor* OwnerRef = nullptr;
	
};
