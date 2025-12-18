// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AEExtractionSpot.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"
#include "Core/AEGlobalHelper.h"
#include "Characters/Player/PlayerCharacter.h"


// Sets default values
AAEExtractionSpot::AAEExtractionSpot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerBox");
	RootComponent = TriggerBox;

	DecalComp = CreateDefaultSubobject<UDecalComponent>("DecalComponent");
	DecalComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAEExtractionSpot::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAEExtractionSpot::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AAEExtractionSpot::OnOverlapEnd);
}

void AAEExtractionSpot::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* CharacterRef = Cast<APlayerCharacter>(OtherActor);
	if (!CharacterRef) return;

	CharacterPtr = CharacterRef;

	GetWorldTimerManager().SetTimer(ExtractionTimerHandle, this, &AAEExtractionSpot::OnExtractionTimeEnded, TimeToExtract, false);

	K2_OnOverlapBegin();
}

void AAEExtractionSpot::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* CharacterRef = Cast<APlayerCharacter>(OtherActor);
	if (!CharacterRef) return;

	if (CharacterPtr.Get() != CharacterRef) return;

	if (ExtractionTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ExtractionTimerHandle);
	}

	if (CharacterPtr.IsValid())
	{
		CharacterPtr.Reset();
	}

	K2_OnOverlapEnd();
}

void AAEExtractionSpot::OnExtractionTimeEnded()
{
	if (!CharacterPtr.IsValid()) return;

	if (ExtractionTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ExtractionTimerHandle);
	}

	if (CharacterPtr.IsValid())
	{
		UAEGlobalHelper::BroadcastExtractEvent(
			this,
			CharacterPtr.Get(),
			this,
			ExtractionSpotTags
		);

		CharacterPtr.Reset();
		K2_OnExtractionTimeEnded();
	}
}