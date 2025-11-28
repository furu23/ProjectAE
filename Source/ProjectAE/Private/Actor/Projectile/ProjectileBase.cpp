// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"

AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false; // 투사체는 틱 돌리지 마세요 (성능 최적화)

    // 1. 충돌체 설정
    SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
    SetRootComponent(SphereComp);
    SphereComp->SetCollisionProfileName("Projectile"); // Projectile 전용 채널 추천
    SphereComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);

    // 2. 무브먼트 설정
    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
    MovementComp->InitialSpeed = 2000.f;
    MovementComp->MaxSpeed = 2000.f;
    MovementComp->ProjectileGravityScale = 0.f; // 직선 탄도 (중력 무시)

    ProjectileEffect = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
    ProjectileEffect->SetupAttachment(RootComponent);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(3.0f); // 3초 뒤 자동 삭제 (메모리 누수 방지)

    if (GetInstigator())
    {
        GetInstigator()->MoveIgnoreActorAdd(this);
        SphereComp->IgnoreActorWhenMoving(GetInstigator(), true);
    }
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this) return;

    // GAS 적용
    if (HasAuthority())
    {
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

        if (TargetASC && DamageEffectSpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
        }
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