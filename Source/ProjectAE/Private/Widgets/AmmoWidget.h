// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "AmmoWidget.generated.h"

class UAbilitySystemComponent;
class UAEWeaponComponent;
class UTextBlock;
class UProgressBar;
class UImage;
class UWidgetAnimation;

UCLASS(Abstract)
class UAmmoWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitializeWidget(UAEWeaponComponent* WeaponComp, UAbilitySystemComponent* ASC);

protected:
	// 콜백 및 내부 non virtual 함수
	UFUNCTION()
	void UpdateAmmoState(int32 NewValue, int32 OldValue);

	void OnReloadStateChanged(FGameplayTag ReloadTag, int32 Level);


	// NVI API 순수 가상 함수
	virtual void NativeOnAmmoUpdated(int32 NewValue, int32 OldValue) {}

	virtual void NativeOnBind() {}

	virtual void OnReloadStarted() {};

	virtual void OnReloadEnded() {};


	// BP 훅 API 함수들
	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnInitialized"))
	void K2_OnInitialized(UAEWeaponComponent* WeaponComp, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnReloadStateStarted"))
	void K2_OnReloadStateStarted(UAEWeaponComponent* WeaponComp, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnReloadStateEnded"))
	void K2_OnReloadStateEnded(UAEWeaponComponent* WeaponComp, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = (DisplayName = "OnAmmoAmountChanged"))
	void K2_OnAmmoAmountChanged(int32 NewValue, int32 OldValue);

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmmo = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	int32 MaxAmmo = -1;

private:
	UAEWeaponComponent* TryRefreshAmmoComp();

	TWeakObjectPtr<UAEWeaponComponent> CachedWeaponComponent;

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};

UCLASS()
class UAmmoWidget : public UAmmoWidgetBase
{
	GENERATED_BODY()


protected:
	virtual void NativeOnBind() override;

	virtual void NativeOnAmmoUpdated(int32 NewValue, int32 OldValue) override;

	virtual void OnReloadStarted() override;

	virtual void OnReloadEnded() override;

	void UpdateVisuals();

	UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_CurrentAmmo;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_MaxAmmo;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> Bar_AmmoGauge;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_WeaponIcon;

	UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_Reload;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> Anim_Reload;
};