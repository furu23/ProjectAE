// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

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
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(3.0f); // 3초 뒤 자동 삭제 (메모리 누수 방지)
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 나 자신이나 나를 쏜 사람(Instigator)과는 충돌 무시
    if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this) return;

    // [핵심] 서버에서만 데미지 적용 (GAS 권한)
    if (HasAuthority())
    {
        // 1. 맞은 놈의 ASC 찾기
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

        // 2. 배달온 소포(Spec) 적용
        if (TargetASC && DamageEffectSpecHandle.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
        }
    }

    // 3. 시각 효과 (클라이언트) - 폭발 이펙트 등

    // 4. 임무 완수 후 자폭
    Destroy();
}