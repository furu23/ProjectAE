// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/InteractionComponent.h"

#include "Interfaces/Interactable.h"
#include "Characters/BaseCharacter.h"
#include "Interaction/BaseInteractableActor.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySystemComponent.h"


// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerRef = GetOwner();
	if (!OwnerRef) return;
	
	BeginTracing();
}

void UInteractionComponent::TryInteract()
{
	if (!CurrentFocus) return;

	if (IInteractable::Execute_CanInteract(CurrentFocus))
	{
		IInteractable::Execute_OnInteract(CurrentFocus, OwnerRef);
	}
}

void UInteractionComponent::BeginTracing()
{
	if (TraceTimerHandle.IsValid()) { StopTracing(); }
	
	GetWorld()->GetTimerManager().SetTimer(TraceTimerHandle, this, &UInteractionComponent::UpdateTracing, TraceInterval, true);
}

void UInteractionComponent::StopTracing()
{
	if (!TraceTimerHandle.IsValid()) return;
	
	GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
}

void UInteractionComponent::UpdateTracing()
{
	FVector TraceStart, TraceEnd;
	if (!GetTracePoint(TraceStart, TraceEnd)) return;

	if (AActor* HitActor = PerformTrace(TraceStart, TraceEnd))
	{
		if (HitActor != CurrentFocus)
		{
			if (CurrentFocus)
			{
				IInteractable::Execute_OnFocusChanged(CurrentFocus, OwnerRef, false);
			}

			CurrentFocus = HitActor;
			IInteractable::Execute_OnFocusChanged(CurrentFocus, OwnerRef, true);
			OnFocusChanged.Broadcast(CurrentFocus);

			// **** [추가] Ability Grant 관련 ****

			UE_LOG(LogTemp, Log, TEXT("---Pull and Grant Ability on Actor---"));

			ABaseCharacter* CastedCharacter = GetOwner<ABaseCharacter>();
			if (CastedCharacter)
			{
				UAbilitySystemComponent* OwnerASC = CastedCharacter->GetASC();
				ABaseInteractableActor* TargetActor = Cast<ABaseInteractableActor>(CurrentFocus);
				if (OwnerASC && TargetActor->Implements<UInteractable>())
				{
					TSubclassOf<UGameplayAbility> AbilityToGrant = TargetActor->GrantAbility;
					if (AbilityToGrant)
					{	
						FGameplayAbilitySpec Spec(AbilityToGrant, 1, -1, TargetActor);

						GrantedAbilityHandle = OwnerASC->GiveAbility(Spec);
						if (GrantedAbilityHandle.IsValid())
						{
							UE_LOG(LogTemp, Log, TEXT("OnBeginFocus: GiveAbility SUCCESS. Handle: %s"), *GrantedAbilityHandle.ToString());
						}
						else
						{
							// 이 로그가 뜬다면 ASC 내부에서 부여가 실패한 것
							UE_LOG(LogTemp, Error, TEXT("OnBeginFocus: GiveAbility FAILED. Returned Invalid Handle."));
						}
						UE_LOG(LogTemp, Log, TEXT("Granted On ABaseCharacter"));
						UE_LOG(LogTemp, Log, TEXT("-------------------------"));
					}
				}
			}
		}
	}
	else
	{
		if (CurrentFocus)
		{
			IInteractable::Execute_OnFocusChanged(CurrentFocus, OwnerRef, false);
			CurrentFocus = nullptr;
			OnFocusChanged.Broadcast(CurrentFocus);

			ABaseCharacter* CastedCharacter = GetOwner<ABaseCharacter>();
			if (CastedCharacter)
			{
				UAbilitySystemComponent* OwnerASC = CastedCharacter->GetASC();
				if (OwnerASC && GrantedAbilityHandle.IsValid())
				{
					OwnerASC->SetRemoveAbilityOnEnd(GrantedAbilityHandle);
				}
			}
		}
	}
}

AActor* UInteractionComponent::PerformTrace(const FVector& TraceStart, const FVector& TraceEnd)
{
	FHitResult Hit;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(OwnerRef);
	
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannel, TraceParams);
	if (!bHit) return nullptr;

	if (AActor* HitActor = Hit.GetActor())
	{
		if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
		{
			if (IInteractable::Execute_CanFocus(HitActor))
			{
				return HitActor;
			}
		}
	}
	
	return nullptr;
}

bool UInteractionComponent::GetTracePoint(FVector& OutStart, FVector& OutEnd)
{
	if (!OwnerRef) return false;
	
	APlayerController* PC = Cast<APlayerController>(OwnerRef->GetInstigatorController());
	if (!PC) return false;

	FVector WorldLocation;
	FVector WorldDirection;

	if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection)) return false;

	// FVector PlayerLocation = OwnerRef->GetActorLocation();
	// if ((PlayerLocation - WorldLocation).Size() > 1000.f) return false;

	OutStart = WorldLocation;
	OutEnd = OutStart + (WorldDirection * TraceDistance);

	return true;
}
