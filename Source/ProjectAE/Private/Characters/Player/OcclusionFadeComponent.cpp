// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/OcclusionFadeComponent.h"

#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UOcclusionFadeComponent::UOcclusionFadeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts
void UOcclusionFadeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UOcclusionFadeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TimeSinceLastUpdate += DeltaTime;
	
	if (TimeSinceLastUpdate < MinUpdateTime) return;
	
	TSet<UPrimitiveComponent*> HitComponents;
	
	TracingOcclusionObjects(HitComponents);
	ProcessFadingAndCleanUp(HitComponents, TimeSinceLastUpdate);
	
	TimeSinceLastUpdate = 0.0f;
}

void UOcclusionFadeComponent::TracingOcclusionObjects(TSet<UPrimitiveComponent*>& OutHitComponents)
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	
	if (!CameraManager || !PlayerPawn) return;
	
	FVector Start = CameraManager->GetCameraLocation();
	FVector End = PlayerPawn->GetActorLocation();
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerPawn);
	
	bool bHit = GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, OcclusionChannel, Params);
	if (!bHit) return;
	
	for (const FHitResult Hit : HitResults)
	{
		UPrimitiveComponent* HitComponent = Hit.GetComponent();
		if (HitComponent)
		{
			OutHitComponents.Add(HitComponent);
		}
	}
}

void UOcclusionFadeComponent::ProcessFadingAndCleanUp(const TSet<UPrimitiveComponent*>& HitComponents, float DeltaTime)
{
	for (auto It = FadingObjects.CreateIterator(); It; ++It)
	{
		FFadingObject& Object = It.Value();
		UPrimitiveComponent* ObjectComponent = Object.PrimitiveComp.Get();
		if (!ObjectComponent)
		{
			It.RemoveCurrent();
			continue;
		}
		
		// 목표 값 설정
		// HitComponents 에 포함 중 -> 가리는 중 -> OccludedTargetValue
		if (HitComponents.Contains(ObjectComponent))
		{
			Object.TargetOpacity = OccludedTargetValue;
		}
		else
		{
			Object.TargetOpacity = ClearTargetValue;
		}
		
		// 값 보간
		Object.CurrentOpacity = FMath::FInterpTo(Object.CurrentOpacity, Object.TargetOpacity, DeltaTime, FadeSpeed);
		
		// CPD 값 적용
		ObjectComponent->SetCustomPrimitiveDataFloat(CPDIndex, Object.CurrentOpacity);
		
		if (FMath::IsNearlyEqual(Object.TargetOpacity, ClearTargetValue) && FMath::IsNearlyEqual(Object.CurrentOpacity, ClearTargetValue, 0.1f))
		{
			ObjectComponent->SetCustomPrimitiveDataFloat(CPDIndex, ClearTargetValue);
			It.RemoveCurrent();
		}
	}

	// 새로운 객체 등록
	for (UPrimitiveComponent* HitComponent : HitComponents)
	{
		if (!FadingObjects.Contains(HitComponent))
		{
			FFadingObject NewObject(HitComponent, ClearTargetValue);
			NewObject.TargetOpacity = OccludedTargetValue;
			
			// FadingObjects.Add(HitComponent, NewObject);
			FadingObjects.Add(TWeakObjectPtr<UPrimitiveComponent>(HitComponent), NewObject);
		}
	}
}
