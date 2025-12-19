// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AEQuestLocationActor.h"
#include "Characters/Player/PlayerCharacter.h"
#include "QuestMessageHelpers.h"
#include "Components/BoxComponent.h"
#include "GameplayTagContainer.h"

AAEQuestLocationActor::AAEQuestLocationActor()
{
	BoxComp = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	RootComponent = BoxComp;
}

void AAEQuestLocationActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) { return; }

	NotifyActorBeginOverlap(OtherActor);
}

void AAEQuestLocationActor::BeginPlay()
{
	// UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	// if (GameInstance)
	// {
	// 	USaveGameSubsystem* SaveSys = GameInstance->GetSubsystem<USaveGameSubsystem>();
	// 	if (SaveSys && SaveSys->IsEventCompleted(QuestEventTag))
	// 	{
	// 		this->Destroy();
	// 		return;
	// 	}
	// }

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AAEQuestLocationActor::OnOverlapBegin);
}

void AAEQuestLocationActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);


	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player && QuestEventTag.IsValid())
	{
		UQuestMessageHelpers::BroadcastLocationEvent(this, OtherActor, this, FGameplayTagContainer(QuestEventTag));

		// UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
		// if (GameInstance)
		// {
		// 	USaveGameSubsystem* SaveSys = GameInstance->GetSubsystem<USaveGameSubsystem>();
		// 	if (SaveSys)
		// 	{
		// 		SaveSys->MarkEventCompleted(QuestEventTag);
		// 	}
		// }
	}
}
