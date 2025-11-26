// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AEQuestLocationActor.h"
#include "Characters/Player/PlayerCharacter.h"
#include "QuestMessageHelpers.h"
#include "Components/BoxComponent.h"

AAEQuestLocationActor::AAEQuestLocationActor()
{
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	RootComponent = BoxComp;
}

void AAEQuestLocationActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bIsAlreadyTriggered)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
		if (Player && ObjectiveTags.IsValid())
		{
			UQuestMessageHelpers::BroadcastLocationEvent(this, OtherActor, this, ObjectiveTags);
			bIsAlreadyTriggered = true;
		}
	}
}
