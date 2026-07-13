// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AEGameMode.generated.h"

class FRenderCommandFence;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnLandscapeGrassReady,
	bool, bTimedOut,
	float, ElapsedSeconds);

UENUM(BlueprintType)
enum class ELandscapeGrassStartupPolicy : uint8
{
	/** Baseline: request asynchronous grass regeneration without changing its priority or preloading RVT pages. */
	RegenerateOnly UMETA(DisplayName = "Regenerate Only (Baseline)"),

	/** Optimized: prioritize camera-near grass and preload the RVT ranges used by the initial view. */
	PrioritizedNearField UMETA(DisplayName = "Prioritized Near Field (Optimized)")
};

/**
 * 
 */
UCLASS()
class PROJECTAE_API AAEGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAEGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Fired after grass render data and the preloaded camera-area RVT pages pass the render fence. */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAE|Landscape Grass")
	FOnLandscapeGrassReady OnLandscapeGrassReady;

	UFUNCTION(BlueprintPure, Category = "ProjectAE|Landscape Grass")
	bool IsLandscapeGrassReady() const { return bLandscapeGrassReady; }

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Starts Landscape Grass regeneration and readiness observation around the initial camera. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass")
	bool bPrimeLandscapeGrassOnStart = true;

	/** Startup policy is read once when grass priming begins. Change the BP class default between PIE runs for A/B captures. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass")
	ELandscapeGrassStartupPolicy LandscapeGrassStartupPolicy = ELandscapeGrassStartupPolicy::PrioritizedNearField;

	/** Maximum time to keep a loading/fade gate closed if render readiness never settles. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0.1"))
	float LandscapeGrassReadyTimeout = 30.0f;

	/** Number of identical grass snapshots required before inserting the render fence. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "1"))
	int32 LandscapeGrassStableFrames = 4;

	/** Only grass component bounds intersecting this horizontal camera radius gate the initial reveal. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0.0"))
	float LandscapeGrassReadyRadius = 4000.0f;

	/** Extra frames after the render fence for RVT page-table visibility and presentation. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0"))
	int32 LandscapeGrassPostFenceFrames = 8;

	/** Highest-resolution RVT preload radius around the initial camera. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0.0"))
	float LandscapeGrassRVTInnerPreloadRadius = 2000.0f;

	/** RVT mip requested for the inner preload area. Zero is the highest-resolution mip. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0"))
	int32 LandscapeGrassRVTInnerPreloadMip = 0;

	/** Wider, lower-resolution RVT preload radius around the initial camera. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0.0"))
	float LandscapeGrassRVTOuterPreloadRadius = 4000.0f;

	/** RVT mip requested for the outer preload area. */
	UPROPERTY(EditDefaultsOnly, Category = "ProjectAE|Landscape Grass", meta = (ClampMin = "0"))
	int32 LandscapeGrassRVTOuterPreloadMip = 1;

	void PrimeLandscapeGrass();
	void PollLandscapeGrassReadiness();
	void RequestLandscapeGrassRVTPreload() const;
	void SetLandscapeGrassPriority(bool bPrioritize);
	void CompleteLandscapeGrassReadiness(bool bTimedOut);
	bool UsesPrioritizedNearFieldPolicy() const;

	void OnExtractionEvent(FGameplayTag Channel, const FQuestMessage_Generic& Payload);

private:
	TSharedPtr<FRenderCommandFence> LandscapeGrassRenderFence;
	FVector LandscapeGrassCameraLocation = FVector::ZeroVector;
	double LandscapeGrassPrimeStartSeconds = 0.0;
	int32 LandscapeGrassLastNearComponentCount = INDEX_NONE;
	int64 LandscapeGrassLastNearRenderInstanceCount = INDEX_NONE;
	int32 LandscapeGrassStableFrameCount = 0;
	int32 LandscapeGrassFramesAfterFence = 0;
	uint64 LandscapeGrassTraceRegionId = 0;
	bool bLandscapeGrassReady = false;
	bool bLandscapeGrassReadinessActive = false;
	bool bLandscapeGrassPriorityActive = false;
	bool bLandscapeGrassUsesPrioritizedNearFieldPolicy = false;

	FGameplayMessageListenerHandle ExtractionListenerHandle;
};
