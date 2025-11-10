// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class PROJECTAE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

protected:

	// *** 어빌리티 시스템 관련 기능 ***
	// 추후, 하위 클래스 작성 시 옮기거나 확장할 수 있습니다.

	// 어빌리티 시스템 초기화
	void InitAbiltySystem();

	// 클라이언트에서 플레이어 상태가 복제될 때 호출되는 함수 (어빌리티 시스템 초기화에 사용)
	void OnRep_PlayerState() override;

	// 어빌리티 시스템 컴포넌트 캐시
	UPROPERTY()
	TWeakObjectPtr<class UAbilitySystemComponent> CachedASC;
};
