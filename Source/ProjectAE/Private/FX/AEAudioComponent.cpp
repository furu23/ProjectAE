// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/AEAudioComponent.h"

UAEAudioComponent::UAEAudioComponent()
{
	bAutoActivate = true;
	bAllowSpatialization = true;
	bOverrideAttenuation = true;
}

void UAEAudioComponent::BeginPlay()
{
	// 에디터에서 Attenuation 에셋을 안 넣었으면 경고 로그를 띄웁니다.
	if (!AttenuationSettings)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[Warning] %s: Attenuation Asset is MISSING!"), *GetName()));

		UE_LOG(LogTemp, Warning, TEXT("%s: Please assign Attenuation Settings in Blueprint Details Panel."), *GetName());
	}

	if (ConcurrencySet.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No Concurrency Set assigned. Footsteps might overlap!"), *GetName());
	}
}

void UAEAudioComponent::TriggerFootstep()
{
	Super::BeginPlay();

	if (IsActive())
	{
		SetTriggerParameter(FName("OnPlay"));
	}
	else
	{
		Play();
		SetTriggerParameter(FName("OnPlay"));
	}
}
