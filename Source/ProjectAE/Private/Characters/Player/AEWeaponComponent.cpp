// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AEWeaponComponent.h"
#include "AbilitySystemComponent.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Characters/Player/Data/AEWeaponDefinition.h"
#include "ProjectAE/ProjectAE.h"

void UAEWeaponComponent::EquipWeapon(UAEWeaponDefinition* NewWeaponDef)
{
    if (!NewWeaponDef) return;
    // if (CurrentWeaponDef == NewWeaponDef) return; // 이미 장착 중

    APlayerCharacter* PlayerRef = Cast<APlayerCharacter>(GetOwner());
    if (!PlayerRef) return;

    UAbilitySystemComponent* ASC = PlayerRef->GetASC();

    // 이전 정의 및 부여 능력 제거
    if (CurrentWeaponDef)
    {
        if (ASC)
        {
            for (FGameplayAbilitySpecHandle CurrentHandle : CurrentWeaponAbilityHandles)
            {
                if (!CurrentHandle.IsValid())
                {
                    UE_LOG(LogAbilitySys, Error, TEXT("EquipWeapon : InValid Handle Detected In Player Character."));
                }
                ASC->ClearAbility(CurrentHandle); // 이전 능력 제거

                CurrentWeaponAbilityHandles.Empty();
            }
        }
    }

    // 새로운 무기 정의 저장 및 기본 초기화
    CurrentWeaponDef = NewWeaponDef;
    CurrentAmmo = CurrentWeaponDef->MaxAmmo; // 탄약 채우기

    // 어빌리티 부여
    if (ASC)
    {
        // GA를 부여하고 핸들을 저장해둠
        for (const TSubclassOf<UGameplayAbility>& AbilitySet : NewWeaponDef->WeaponAbilities)
        {
            if (!AbilitySet) continue;

            FGameplayAbilitySpec NewSpec(AbilitySet, 1, -1, this);
            FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(NewSpec);
            CurrentWeaponAbilityHandles.Add(Handle);
        }
    }

    // 추가할 로직이 있다면 여기에
}

void UAEWeaponComponent::ReloadWeapon()
{
    const int32 OldAmmo = CurrentAmmo;

    CurrentAmmo = GetMaxAmmo();

    if (OnAmmoUseDelegate.IsBound())
    {
        OnAmmoUseDelegate.Broadcast(CurrentAmmo, OldAmmo);
    }
}

bool UAEWeaponComponent::UseAmmo(int32 AmmoUseAmount)
{
    int32 NewAmmo = CurrentAmmo - AmmoUseAmount;
    if (NewAmmo < 0) { return false; }


	CurrentAmmo = NewAmmo;
    if (OnAmmoUseDelegate.IsBound())
    {
        OnAmmoUseDelegate.Broadcast(GetCurrentAmmo(), GetCurrentAmmo() + AmmoUseAmount);
    }
    return true;
}

int32 UAEWeaponComponent::GetMaxAmmo() const
{
    if (CurrentWeaponDef)
    {
        return CurrentWeaponDef->MaxAmmo;
    }
    return 0;
}

