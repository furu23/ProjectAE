#include "Widgets/AmmoWidget.h"
#include "AbilitySystemComponent.h"
#include "Characters/Player/PlayerCharacter.h"
#include "ProjectAE/ProjectAE.h"
#include "Characters/Player/AEWeaponComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UAmmoWidgetBase::InitializeWidget(UAEWeaponComponent* WeaponComp, UAbilitySystemComponent* ASC)
{
    CachedWeaponComponent = WeaponComp;
    CachedASC = ASC;

    if (!ASC && !WeaponComp)
    {
        UE_LOG(LogAbilitySys, Error, TEXT("[AmmoWidget] Initailize failed for Invalid ASC argument."));
        return;
    }

    ASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Action.Reloading"), EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &UAmmoWidgetBase::OnReloadStateChanged);

    WeaponComp->OnAmmoUseDelegate.AddDynamic(this, &UAmmoWidgetBase::UpdateAmmoState);

    UpdateAmmoState(WeaponComp->GetCurrentAmmo(), CurrentAmmo);
    MaxAmmo = WeaponComp->GetMaxAmmo();

    NativeOnBind();
    K2_OnInitialized(WeaponComp, ASC);
}

void UAmmoWidgetBase::UpdateAmmoState(int32 NewValue, int32 OldValue)
{
    CurrentAmmo = NewValue;

    NativeOnAmmoUpdated(NewValue, OldValue);
    K2_OnAmmoAmountChanged(NewValue, OldValue);
}

void UAmmoWidgetBase::OnReloadStateChanged(FGameplayTag ReloadTag, int32 Level)
{
    if (!CachedASC.IsValid())
    {
        UE_LOG(LogAbilitySys, Error, TEXT("[AmmoWidget] Initailize failed for Invalid ASC argument."));
        return;
    }
    if (!CachedWeaponComponent.IsValid() && !TryRefreshAmmoComp())
    {
        return;
    }

    if (Level == 0)
    {
        OnReloadEnded();
        K2_OnReloadStateEnded(CachedWeaponComponent.Get(), CachedASC.Get());
    }
    else
    {
        OnReloadStarted();
        K2_OnReloadStateStarted(CachedWeaponComponent.Get(), CachedASC.Get());
    }
}

UAEWeaponComponent * UAmmoWidgetBase::TryRefreshAmmoComp()
{
    if (CachedWeaponComponent.IsValid())
    {
        return CachedWeaponComponent.Get();
    }

    UAbilitySystemComponent* ASC = CachedASC.Get();
    if (!ASC) return nullptr;

    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(ASC->GetAvatarActor());
    if (!PlayerChar) return nullptr;

    UAEWeaponComponent* FoundComp = PlayerChar->GetWeaponComponent();

    if (FoundComp)
    {
        CachedWeaponComponent = FoundComp;

        UE_LOG(LogAbilitySys, Warning, TEXT("[AmmoWidget] WeaponComp was missing but recovered manually. Actor: %s. Check InitializeWidget timing!"),
            *GetOwningPlayerPawn()->GetName());

        return FoundComp;
    }

    UE_LOG(LogAbilitySys, Error, TEXT("[AmmoWidget] WeaponComp was missing and Failed to find manually!"));

    return nullptr;
}

void UAmmoWidget::NativeOnBind()
{
    if (Text_MaxAmmo)
    {
        Text_MaxAmmo->SetText(FText::AsNumber(MaxAmmo));
    }

    UpdateVisuals();
}

void UAmmoWidget::NativeOnAmmoUpdated(int32 NewValue, int32 OldValue)
{
    UpdateVisuals();
}

void UAmmoWidget::OnReloadStarted()
{
    if (Image_Reload)
    {
        Image_Reload->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    if (Anim_Reload)
    {
        PlayAnimation(Anim_Reload, 0.0f, 0);
    }
}

void UAmmoWidget::OnReloadEnded()
{
    if (Anim_Reload)
    {
        StopAnimation(Anim_Reload);
    }

    if (Image_Reload)
    {
        Image_Reload->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UAmmoWidget::UpdateVisuals()
{
    if (Text_CurrentAmmo)
    {
        Text_CurrentAmmo->SetText(FText::AsNumber(CurrentAmmo));
    }

    if (Bar_AmmoGauge && MaxAmmo > 0)
    {
        const float Percent = (float)CurrentAmmo / (float)MaxAmmo;
        Bar_AmmoGauge->SetPercent(Percent);
    }
}
