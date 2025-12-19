// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/BaseEnemyCharacter.h"
#include "AbilitySystem/AEAbilitySystemComponent.h"
#include "AbilitySystem/AS_HealthSet.h"
#include "AbilitySystem/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "QuestMessageHelpers.h"


UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	if (ASC)
	{
		return ASC;
	}
	return nullptr;
}

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	ASC = CreateDefaultSubobject<UAEAbilitySystemComponent>("ASC");

	HealthComp = CreateDefaultSubobject<UHealthComponent>("HealthComp");
	HealthSet = CreateDefaultSubobject<UAS_HealthSet>("HealthSet");

	// AttributeSet 등록
	ASC->AddAttributeSetSubobject<UAS_HealthSet>(HealthSet);
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnDeathDelegate.AddDynamic(this, &ABaseEnemyCharacter::OnDeath);
		HealthComp->OnDamageDelegate.AddDynamic(this, &ABaseEnemyCharacter::OnDamaged);
	}
}

void ABaseEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HealthComp)
	{
		HealthComp->TryInitAbilitySystem(ASC);
	}
}

void ABaseEnemyCharacter::OnDeath_Implementation(AActor* Causer, AActor* Victim)
{
	// 1) 컨트롤러 분리 및 이동 정지
	if (AController* APC = GetController())
	{
		// AI나 플레이어 제어 해제
		APC->Destroyed();
	}
	DetachFromControllerPendingDestroy();

	if (UCharacterMovementComponent* CharMove = GetCharacterMovement())
	{
		// 이동을 완전히 비활성화
		CharMove->StopMovementImmediately();
		CharMove->DisableMovement();
	}

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		// 메쉬를 루트(캡슐)로부터 분리하여 월드에 독립시킵니다.
		SkelMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	// 2) 캡슐 충돌 비활성화 (ragdoll과 충돌 방지)
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	// 3) 메쉬를 ragdoll로 전환하여 자연스럽게 쓰러지게 함
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		// 충돌 프로필을 ragdoll로 설정(프로젝트에 프로필이 없으면 QueryAndPhysics 사용)
		SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));

		// 물리 시뮬레이션 시작
		SkelMesh->SetAllBodiesSimulatePhysics(true);
		SkelMesh->SetSimulatePhysics(true);
		SkelMesh->WakeAllRigidBodies();
		SkelMesh->bBlendPhysics = true; // 애니와 물리 블렌드 허용(자연스러운 전환)

		// 현재 물리 속도가 너무 크면 클램프하여 날아가는 현상 방지
		const float MaxLinearSpeed = 600.0f; // 필요 시 튜닝
		FVector CurrentLinear = SkelMesh->GetPhysicsLinearVelocity(NAME_None);
		if (CurrentLinear.Size() > MaxLinearSpeed)
		{
			SkelMesh->SetAllPhysicsLinearVelocity(CurrentLinear.GetClampedToMaxSize(MaxLinearSpeed));
		}

		// 각속도 제거하여 불필요한 회전 최소화
		SkelMesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		// 감쇄 설정으로 빠르게 안정화
		SkelMesh->SetLinearDamping(4.0f);
		SkelMesh->SetAngularDamping(4.0f);

		// 약한 임펄스를 주면 더 자연스럽게 넘어짐 (Causer가 있으면 그 반대 방향으로 가볍게)
		if (Causer)
		{
			FVector ImpulseDir = (SkelMesh->GetComponentLocation() - Causer->GetActorLocation()).GetSafeNormal();
			const float ImpulseStrength = 250.0f; // 필요 시 튜닝
			SkelMesh->AddImpulse(ImpulseDir * ImpulseStrength, NAME_None, true);
		}
	}

	// 4) 사망 처리 후 기존 로직
	SetLifeSpan(10.0f);

	UQuestMessageHelpers::BroadcastAIKilledEvent(this, Causer, Victim, FGameplayTagContainer(CharacterTag));

	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f);

	AActor* SpawnedBioItem = GetWorld()->SpawnActor<AActor>(BioItemClass, SpawnLocation, FRotator::ZeroRotator);

	if (SpawnedBioItem)
	{
		SpawnedBioItem->SetLifeSpan(60.f);
	}
}

void ABaseEnemyCharacter::OnDamaged_Implementation(AActor* Causer, AActor* Victim)
{

}
