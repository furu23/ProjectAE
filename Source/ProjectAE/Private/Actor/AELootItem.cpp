// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AELootItem.h"
#include "Components/BoxComponent.h"
#include "Characters/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"

AAELootItem::AAELootItem()
{
	LootCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LootCollisionBox"));
	SetRootComponent(LootCollisionBox);

	LootCollisionBox->SetCollisionProfileName(TEXT("Trigger"));
	LootCollisionBox->SetGenerateOverlapEvents(true);
}

void AAELootItem::BeginPlay()
{
	Super::BeginPlay();

	if (LootCollisionBox)
	{
		LootCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAELootItem::OnOverlapBegin);
	}
}

void AAELootItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ensure(BioHealEffectClass)) return;

	if (OtherActor && (OtherActor != this))
	{
		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(OtherActor);
		if (PlayerChar)
		{
			
			UAbilitySystemComponent* ASC = PlayerChar->GetASC();
			if (ASC)
			{
				
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				
				const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(BioHealEffectClass, 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.BioHealAmount")), HealAmount);
				}

				ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, ASC);

				this->Destroy();
			}
		}
	}
}