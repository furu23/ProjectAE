// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OcclusionFadeComponent.generated.h"


struct FFadingObject
{
    TWeakObjectPtr<UPrimitiveComponent> PrimitiveComp;
    float CurrentOpacity;
    float TargetOpacity; // 0.0 (반투명) 또는 1.0 (투명)

    FFadingObject(UPrimitiveComponent* InComp, float InOpacity)
        : PrimitiveComp(InComp), CurrentOpacity(InOpacity), TargetOpacity(0.0f) {}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTAE_API UOcclusionFadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOcclusionFadeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	void TracingOcclusionObjects(TSet<UPrimitiveComponent*>& OutHitComponents);
	
	void ProcessFadingAndCleanUp(const TSet<UPrimitiveComponent*>& HitComponents, float DeltaTime);
	
public:
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	TEnumAsByte<ECollisionChannel> OcclusionChannel = ECC_GameTraceChannel3;	// 현재 Transparency 채널 (Default Response = Overlap, 무시할 액터는 Ignore로 따로 설정)
	
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float OccludedTargetValue = 1.0f; // 기본 값: 1.0 (투명)
	
	UPROPERTY(VisibleAnywhere, Category = "Occlusion")
	float ClearTargetValue = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float FadeSpeed = 5.0f;
	
	UPROPERTY(VisibleAnywhere, Category = "Occlusion")
	int32 CPDIndex = 0;
	
private:
	TMap<TWeakObjectPtr<UPrimitiveComponent>, FFadingObject> FadingObjects;
	
	UPROPERTY(VisibleAnywhere, Category = "Occlusion")
	float MinUpdateTime = 0.033f;	// 최소 업데이트 간격 (30FPS 기준 0.033초)
	
	float TimeSinceLastUpdate = 0.0f;
		
};
