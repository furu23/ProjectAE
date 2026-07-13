// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AEGameMode.h"
#include "Core/GamePhaseSubsystem.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "GrassInstancedStaticMeshComponent.h"
#include "LandscapeSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RenderCommandFence.h"
#include "UObject/UObjectIterator.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/MiscTrace.h"

TRACE_DECLARE_INT_COUNTER(AEGrassNearComponents, TEXT("ProjectAE/Grass/Near Components"));
TRACE_DECLARE_INT_COUNTER(AEGrassNearRenderInstances, TEXT("ProjectAE/Grass/Near Render Instances"));
TRACE_DECLARE_INT_COUNTER(AEGrassNearAsyncBuilds, TEXT("ProjectAE/Grass/Near Async Builds"));
TRACE_DECLARE_INT_COUNTER(AEGrassStableFrames, TEXT("ProjectAE/Grass/Stable Frames"));
TRACE_DECLARE_INT_COUNTER(AEGrassPostFenceFrames, TEXT("ProjectAE/Grass/Post Fence Frames"));

namespace
{
	struct FGrassSnapshot
	{
		int32 ComponentCount = 0;
		int32 RegisteredComponentCount = 0;
		int32 RenderStateComponentCount = 0;
		int32 AsyncBuildingComponentCount = 0;
		int64 RenderInstanceCount = 0;

		bool IsReady() const
		{
			return ComponentCount > 0
				&& RegisteredComponentCount == ComponentCount
				&& RenderStateComponentCount == ComponentCount
				&& AsyncBuildingComponentCount == 0
				&& RenderInstanceCount > 0;
		}
	};

	bool IsWithinHorizontalRadius(
		const UGrassInstancedStaticMeshComponent* Component,
		const FVector& Center,
		float Radius)
	{
		const FBoxSphereBounds& Bounds = Component->GetBounds();
		const double DeltaX = FMath::Max(FMath::Abs(Center.X - Bounds.Origin.X) - Bounds.BoxExtent.X, 0.0);
		const double DeltaY = FMath::Max(FMath::Abs(Center.Y - Bounds.Origin.Y) - Bounds.BoxExtent.Y, 0.0);
		const double RadiusSquared = FMath::Square(static_cast<double>(Radius));
		return DeltaX * DeltaX + DeltaY * DeltaY <= RadiusSquared;
	}

	FGrassSnapshot CaptureGrassSnapshot(
		const UWorld* World,
		const FVector* FilterCenter = nullptr,
		float FilterRadius = 0.0f)
	{
		FGrassSnapshot Snapshot;
		if (!World)
		{
			return Snapshot;
		}

		for (TObjectIterator<UGrassInstancedStaticMeshComponent> It; It; ++It)
		{
			const UGrassInstancedStaticMeshComponent* Component = *It;
			if (IsValid(Component) && Component->GetWorld() == World)
			{
				if (FilterCenter && !IsWithinHorizontalRadius(Component, *FilterCenter, FilterRadius))
				{
					continue;
				}

				++Snapshot.ComponentCount;
				Snapshot.RegisteredComponentCount += Component->IsRegistered() ? 1 : 0;
				Snapshot.RenderStateComponentCount += Component->IsRenderStateCreated() ? 1 : 0;
				Snapshot.AsyncBuildingComponentCount += Component->IsAsyncBuilding() ? 1 : 0;
				Snapshot.RenderInstanceCount += Component->GetNumRenderInstances();
			}
		}

		return Snapshot;
	}
}

AAEGameMode::AAEGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AAEGameMode::StartPlay()
{
    Super::StartPlay();

	PrimeLandscapeGrass();

    // 월드 시스템 가져오기
    UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>();

    // 로비 상태 선언 및 방송
    if (PhaseSys)
    {
        PhaseSys->StartPhaseMonitoring();
    }
}

void AAEGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bLandscapeGrassReadinessActive)
	{
		PollLandscapeGrassReadiness();
	}
}

void AAEGameMode::PrimeLandscapeGrass()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AAEGameMode::PrimeLandscapeGrass);

	if (!bPrimeLandscapeGrassOnStart)
	{
		return;
	}

	UWorld* World = GetWorld();
	ULandscapeSubsystem* LandscapeSubsystem = World ? World->GetSubsystem<ULandscapeSubsystem>() : nullptr;
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (!LandscapeSubsystem || !PlayerController)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[LandscapeGrassPrime] Skipped: LandscapeSubsystem=%s, PlayerController=%s"),
			LandscapeSubsystem ? TEXT("valid") : TEXT("null"),
			PlayerController ? TEXT("valid") : TEXT("null"));
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	LandscapeGrassCameraLocation = CameraLocation;
	LandscapeGrassPrimeStartSeconds = FPlatformTime::Seconds();
	bLandscapeGrassReady = false;
	bLandscapeGrassReadinessActive = true;
	LandscapeGrassLastNearComponentCount = INDEX_NONE;
	LandscapeGrassLastNearRenderInstanceCount = INDEX_NONE;
	LandscapeGrassStableFrameCount = 0;
	LandscapeGrassFramesAfterFence = 0;
	LandscapeGrassRenderFence.Reset();
	SetActorTickEnabled(true);

	bLandscapeGrassUsesPrioritizedNearFieldPolicy =
		LandscapeGrassStartupPolicy == ELandscapeGrassStartupPolicy::PrioritizedNearField;
	const bool bUsesPrioritizedNearField = UsesPrioritizedNearFieldPolicy();
	const TCHAR* PolicyName = bUsesPrioritizedNearField
		? TEXT("PrioritizedNearField")
		: TEXT("RegenerateOnly");
	const TCHAR* TraceRegionName = bUsesPrioritizedNearField
		? TEXT("ProjectAE Grass Readiness - Optimized")
		: TEXT("ProjectAE Grass Readiness - Baseline");

	if (LandscapeGrassTraceRegionId != 0)
	{
		TRACE_END_REGION_WITH_ID(LandscapeGrassTraceRegionId);
		LandscapeGrassTraceRegionId = 0;
	}
	LandscapeGrassTraceRegionId = TRACE_BEGIN_REGION_WITH_ID(TraceRegionName, TEXT("ProjectAE"));

	TRACE_COUNTER_SET(AEGrassNearComponents, 0);
	TRACE_COUNTER_SET(AEGrassNearRenderInstances, 0);
	TRACE_COUNTER_SET(AEGrassNearAsyncBuilds, 0);
	TRACE_COUNTER_SET(AEGrassStableFrames, 0);
	TRACE_COUNTER_SET(AEGrassPostFenceFrames, 0);
	TRACE_BOOKMARK(
		TEXT("ProjectAE Grass Start Policy=%s Radius=%.0f"),
		PolicyName,
		LandscapeGrassReadyRadius);

	if (bUsesPrioritizedNearField)
	{
		SetLandscapeGrassPriority(true);
	}

	TArray<FVector> CameraLocations;
	CameraLocations.Add(CameraLocation);
	TArrayView<FVector> CameraView = MakeArrayView(CameraLocations);

	const FGrassSnapshot NearBefore = CaptureGrassSnapshot(
		World,
		&LandscapeGrassCameraLocation,
		LandscapeGrassReadyRadius);
	const double PriorityPassStartSeconds = FPlatformTime::Seconds();

	// Both A/B policies issue the same asynchronous regeneration request. The optimized
	// policy differs only by camera-near priority and RVT preloading.
	LandscapeSubsystem->RegenerateGrass(
		/* bInFlushGrass = */ false,
		/* bInForceSync = */ false,
		TOptional<TArrayView<FVector>>(CameraView));

	const double PriorityPassMilliseconds = (FPlatformTime::Seconds() - PriorityPassStartSeconds) * 1000.0;
	const FGrassSnapshot NearAfter = CaptureGrassSnapshot(
		World,
		&LandscapeGrassCameraLocation,
		LandscapeGrassReadyRadius);
	const FGrassSnapshot TotalAfter = CaptureGrassSnapshot(World);
	if (bUsesPrioritizedNearField)
	{
		RequestLandscapeGrassRVTPreload();
	}

	UE_LOG(LogTemp, Display,
		TEXT("[LandscapeGrassPrime] PASS Policy=%s, Camera=%s, %.2f ms, near components %d -> %d, near render instances %lld -> %lld, total components=%d"),
		PolicyName,
		*CameraLocation.ToCompactString(),
		PriorityPassMilliseconds,
		NearBefore.ComponentCount,
		NearAfter.ComponentCount,
		NearBefore.RenderInstanceCount,
		NearAfter.RenderInstanceCount,
		TotalAfter.ComponentCount);
}

void AAEGameMode::PollLandscapeGrassReadiness()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AAEGameMode::PollLandscapeGrassReadiness);

	UWorld* World = GetWorld();
	if (!World)
	{
		CompleteLandscapeGrassReadiness(true);
		return;
	}

	const float ElapsedSeconds = static_cast<float>(FPlatformTime::Seconds() - LandscapeGrassPrimeStartSeconds);
	if (ElapsedSeconds >= LandscapeGrassReadyTimeout)
	{
		CompleteLandscapeGrassReadiness(true);
		return;
	}

	const FGrassSnapshot NearSnapshot = CaptureGrassSnapshot(
		World,
		&LandscapeGrassCameraLocation,
		LandscapeGrassReadyRadius);
	TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassNearComponents, NearSnapshot.ComponentCount);
	TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassNearRenderInstances, NearSnapshot.RenderInstanceCount);
	TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassNearAsyncBuilds, NearSnapshot.AsyncBuildingComponentCount);

	const bool bSnapshotUnchanged =
		NearSnapshot.ComponentCount == LandscapeGrassLastNearComponentCount
		&& NearSnapshot.RenderInstanceCount == LandscapeGrassLastNearRenderInstanceCount;

	LandscapeGrassLastNearComponentCount = NearSnapshot.ComponentCount;
	LandscapeGrassLastNearRenderInstanceCount = NearSnapshot.RenderInstanceCount;

	if (!NearSnapshot.IsReady() || !bSnapshotUnchanged)
	{
		LandscapeGrassStableFrameCount = 0;
		LandscapeGrassFramesAfterFence = 0;
		LandscapeGrassRenderFence.Reset();
		TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassStableFrames, 0);
		TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassPostFenceFrames, 0);
		if (UsesPrioritizedNearFieldPolicy())
		{
			RequestLandscapeGrassRVTPreload();
		}
		return;
	}

	++LandscapeGrassStableFrameCount;
	TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassStableFrames, LandscapeGrassStableFrameCount);
	if (!LandscapeGrassRenderFence.IsValid())
	{
		if (UsesPrioritizedNearFieldPolicy())
		{
			RequestLandscapeGrassRVTPreload();
		}
		if (LandscapeGrassStableFrameCount >= LandscapeGrassStableFrames)
		{
			LandscapeGrassRenderFence = MakeShared<FRenderCommandFence>();
			LandscapeGrassRenderFence->BeginFence();
			TRACE_BOOKMARK(
				TEXT("ProjectAE Grass RenderDataReady T=%.3f Components=%d Instances=%lld"),
				ElapsedSeconds,
				NearSnapshot.ComponentCount,
				NearSnapshot.RenderInstanceCount);

			UE_LOG(LogTemp, Display,
				TEXT("[LandscapeGrassPrime] NEAR RENDER DATA READY after %.3f s: radius=%.0f, components=%d, render instances=%lld; fence inserted"),
				ElapsedSeconds,
				LandscapeGrassReadyRadius,
				NearSnapshot.ComponentCount,
				NearSnapshot.RenderInstanceCount);
		}
		return;
	}

	if (!LandscapeGrassRenderFence->IsFenceComplete())
	{
		return;
	}

	if (LandscapeGrassFramesAfterFence == 0)
	{
		TRACE_BOOKMARK(TEXT("ProjectAE Grass FenceComplete T=%.3f"), ElapsedSeconds);
	}

	++LandscapeGrassFramesAfterFence;
	TRACE_COUNTER_SET_IF_DIFFERENT(AEGrassPostFenceFrames, LandscapeGrassFramesAfterFence);
	if (LandscapeGrassFramesAfterFence >= LandscapeGrassPostFenceFrames)
	{
		CompleteLandscapeGrassReadiness(false);
	}
}

void AAEGameMode::RequestLandscapeGrassRVTPreload() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AAEGameMode::RequestLandscapeGrassRVTPreload);

	for (TObjectIterator<URuntimeVirtualTextureComponent> It; It; ++It)
	{
		URuntimeVirtualTextureComponent* Component = *It;
		if (IsValid(Component) && Component->GetWorld() == GetWorld() && Component->IsEnabledInScene())
		{
			if (LandscapeGrassRVTInnerPreloadRadius > 0.0f)
			{
				const FVector InnerExtent(LandscapeGrassRVTInnerPreloadRadius);
				const FBoxSphereBounds InnerBounds(FBox(
					LandscapeGrassCameraLocation - InnerExtent,
					LandscapeGrassCameraLocation + InnerExtent));
				Component->RequestPreload(InnerBounds, LandscapeGrassRVTInnerPreloadMip);
			}

			if (LandscapeGrassRVTOuterPreloadRadius > 0.0f)
			{
				const FVector OuterExtent(LandscapeGrassRVTOuterPreloadRadius);
				const FBoxSphereBounds OuterBounds(FBox(
					LandscapeGrassCameraLocation - OuterExtent,
					LandscapeGrassCameraLocation + OuterExtent));
				Component->RequestPreload(OuterBounds, LandscapeGrassRVTOuterPreloadMip);
			}
		}
	}
}

void AAEGameMode::SetLandscapeGrassPriority(bool bPrioritize)
{
	if (bLandscapeGrassPriorityActive == bPrioritize)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (ULandscapeSubsystem* LandscapeSubsystem = World->GetSubsystem<ULandscapeSubsystem>())
		{
			LandscapeSubsystem->PrioritizeGrassCreation(bPrioritize);
			bLandscapeGrassPriorityActive = bPrioritize;
		}
	}
}

void AAEGameMode::CompleteLandscapeGrassReadiness(bool bTimedOut)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AAEGameMode::CompleteLandscapeGrassReadiness);

	const FGrassSnapshot NearSnapshot = CaptureGrassSnapshot(
		GetWorld(),
		&LandscapeGrassCameraLocation,
		LandscapeGrassReadyRadius);
	const FGrassSnapshot TotalSnapshot = CaptureGrassSnapshot(GetWorld());
	const float ElapsedSeconds = static_cast<float>(FPlatformTime::Seconds() - LandscapeGrassPrimeStartSeconds);

	SetLandscapeGrassPriority(false);
	bLandscapeGrassReady = !bTimedOut;
	bLandscapeGrassReadinessActive = false;
	SetActorTickEnabled(false);
	LandscapeGrassRenderFence.Reset();
	if (LandscapeGrassTraceRegionId != 0)
	{
		TRACE_END_REGION_WITH_ID(LandscapeGrassTraceRegionId);
		LandscapeGrassTraceRegionId = 0;
	}

	if (bTimedOut)
	{
		TRACE_BOOKMARK(
			TEXT("ProjectAE Grass Timeout T=%.3f Components=%d Instances=%lld"),
			ElapsedSeconds,
			NearSnapshot.ComponentCount,
			NearSnapshot.RenderInstanceCount);
		UE_LOG(LogTemp, Warning,
			TEXT("[LandscapeGrassPrime] TIMEOUT after %.3f s: near radius=%.0f, near components=%d, registered=%d, render states=%d, async=%d, render instances=%lld, total components=%d"),
			ElapsedSeconds,
			LandscapeGrassReadyRadius,
			NearSnapshot.ComponentCount,
			NearSnapshot.RegisteredComponentCount,
			NearSnapshot.RenderStateComponentCount,
			NearSnapshot.AsyncBuildingComponentCount,
			NearSnapshot.RenderInstanceCount,
			TotalSnapshot.ComponentCount);
	}
	else
	{
		TRACE_BOOKMARK(
			TEXT("ProjectAE Grass Ready T=%.3f Components=%d Instances=%lld"),
			ElapsedSeconds,
			NearSnapshot.ComponentCount,
			NearSnapshot.RenderInstanceCount);
		UE_LOG(LogTemp, Display,
			TEXT("[LandscapeGrassPrime] NEAR READY after %.3f s: policy=%s, radius=%.0f, near components=%d, near render instances=%lld, total components=%d"),
			ElapsedSeconds,
			UsesPrioritizedNearFieldPolicy() ? TEXT("PrioritizedNearField") : TEXT("RegenerateOnly"),
			LandscapeGrassReadyRadius,
			NearSnapshot.ComponentCount,
			NearSnapshot.RenderInstanceCount,
			TotalSnapshot.ComponentCount);
	}

	OnLandscapeGrassReady.Broadcast(bTimedOut, ElapsedSeconds);
}

bool AAEGameMode::UsesPrioritizedNearFieldPolicy() const
{
	return bLandscapeGrassUsesPrioritizedNearFieldPolicy;
}

void AAEGameMode::BeginPlay()
{   
    Super::BeginPlay();

    // 찾을 필요 없이, 그냥 "이 태그 들리면 나한테 알려줘"라고 등록만 함
    UGameplayMessageSubsystem& GMS = UGameplayMessageSubsystem::Get(this);

    ExtractionListenerHandle = GMS.RegisterListener<FQuestMessage_Generic>(
        FGameplayTag::RequestGameplayTag("Quest.Event.Extract"), // 탈출구에서 쏘는 태그
        this,
        &AAEGameMode::OnExtractionEvent
    );
}

void AAEGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bLandscapeGrassReadinessActive = false;
	LandscapeGrassRenderFence.Reset();
	SetLandscapeGrassPriority(false);
	if (LandscapeGrassTraceRegionId != 0)
	{
		TRACE_END_REGION_WITH_ID(LandscapeGrassTraceRegionId);
		LandscapeGrassTraceRegionId = 0;
	}

    if (ExtractionListenerHandle.IsValid())
    {
        ExtractionListenerHandle.Unregister();
    }
    Super::EndPlay(EndPlayReason);
}


void AAEGameMode::OnExtractionEvent(FGameplayTag Channel, const FQuestMessage_Generic& Payload)
{
    AActor* SuccessPlayer = Payload.InstigatorActor;

    UE_LOG(LogTemp, Log, TEXT("GameMode: Extraction Confirmed for %s"), *SuccessPlayer->GetName());

    // 처리가 끝나면 GamePhaseSubsystem에게 페이즈 전환 요청
    UGamePhaseSubsystem* PhaseSys = GetWorld()->GetSubsystem<UGamePhaseSubsystem>();
    if (PhaseSys)
    {
        PhaseSys->SetGamePhase(FGameplayTag::RequestGameplayTag("Game.Phase.PostGame"));
    }
}
