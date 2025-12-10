// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AEExtractionSpot.generated.h"

class UBoxComponent;
class UDecalComponent;
class APlayerCharacter;

UCLASS()
class PROJECTAE_API AAEExtractionSpot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAEExtractionSpot();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "OnOverlapBegin"))
	void K2_OnOverlapBegin();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "OnOverlapEnd"))
	void K2_OnOverlapEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event", meta = (DisplayName = "OnExtractionProceed"))
	void K2_OnExtractionTimeEnded();

	UFUNCTION(BlueprintCallable, Category = "Extraction", meta = (BlueprintPure = "true"))
	FORCEINLINE APlayerCharacter* GetCurrentCharacterPtr() const { return CharacterPtr.Get(); }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void OnExtractionTimeEnded();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UDecalComponent> DecalComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	float TimeToExtract = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction")
	FGameplayTagContainer ExtractionSpotTags = FGameplayTagContainer::EmptyContainer;

private:
	TWeakObjectPtr<APlayerCharacter> CharacterPtr = nullptr;

	FTimerHandle ExtractionTimerHandle;
};
