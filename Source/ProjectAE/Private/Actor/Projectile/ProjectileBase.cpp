// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "FX/Data/HitFeedback.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Core/AEGlobalHelper.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"

AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
    SetRootComponent(SphereComp);
    SphereComp->SetCollisionProfileName("Projectile"); // Projectile 전용 채널 추천
    SphereComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);

    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
    MovementComp->InitialSpeed = 2000.f;
    MovementComp->MaxSpeed = 2000.f;
    MovementComp->ProjectileGravityScale = 0.f;

    ProjectileEffect = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
    ProjectileEffect->SetupAttachment(RootComponent);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(3.0f);

    if (GetInstigator())
    {
        GetInstigator()->MoveIgnoreActorAdd(this);
        SphereComp->IgnoreActorWhenMoving(GetInstigator(), true);
    }
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this) return;

    // 타격 이팩트 적용
    EPhysicalSurface SurfaceType = SurfaceType_Default;
    if (Hit.PhysMaterial.IsValid())
    {
		SurfaceType = Hit.PhysMaterial->SurfaceType;
	}

    SpawnImpactHit(Hit.Location, Hit.Normal, SurfaceType);

    // GAS 적용
    UAEAbilitySystemComponent* TargetASC = UAEGlobalHelper::GetAbilitySystemComponent(OtherActor);

    if (TargetASC && DamageEffectSpecHandle.IsValid())
    {
        TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
    }

    // 시각 효과
    if (ProjectileEffect)
    {
        ProjectileEffect->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        ProjectileEffect->Deactivate();
    }

    SetActorEnableCollision(false);

    if (MovementComp)
    {
        MovementComp->StopMovementImmediately();
    }
}

void AProjectileBase::SpawnImpactHit(FVector Location, FVector Normal, EPhysicalSurface PhysSurf)
{
    if (!ensureMsgf(PhysSurfaceMap, TEXT("No Valid Data Asset In Bullet"))) { return; }

    const FImpactFXInfo* FXInfoToSpawn = PhysSurfaceMap->SurfEffectMap.Find(PhysSurf);

    UNiagaraSystem* EffectToSpawn = FXInfoToSpawn->VisualEffect;
    USoundBase* SoundToSpawn = FXInfoToSpawn->SoundEffect;

    if (EffectToSpawn)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, EffectToSpawn, Location, Normal.Rotation())->Activate();
    }

    if (SoundToSpawn)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToSpawn, Location);
    }
}
