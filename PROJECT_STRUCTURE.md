# Project Folder Structure

## Content 폴더

```text
Content/ProjectAE/
├── AI/
│   ├── Decorators/
│   ├── Enums/
│   └── Tasks/
├── Blueprints/
│   ├── Abilities/
│   │   ├── GameplayAbilities/
│   │   ├── GameplayCue/
│   │   └── GameplayEffects/
│   ├── Actors/
│   │   └── Projectiles/
│   ├── Core/
│   │   ├── BP_AEGameMode
│   │   ├── BP_AEHUD
│   │   └── BP_AEInRaidGameMode
│   └── Interactions/
│       ├── BP_Chest
│       └── BP_Interactable_Base
├── Characters/
│   ├── Enemy/
│   │   ├── Fighter/
│   │   ├── Guner/
│   │   ├── Sneaker/
│   │   └── BP_Enemy_Base
│   └── Player/
│       ├── Animations/
│       ├── BP_AEPlayerController
│       └── BP_Player
├── Data/
│   ├── FX/
│   ├── Input/
│   ├── Items/
│   ├── Quests/
│   └── Weapons/
├── Fonts/
├── FX/
├── Maps/
├── Materials/
├── Textures/
└── Widgets/
```

## Source 폴더

```text
Source/ProjectAE/
├── AbilitySystem/
│   ├── Abilities/
│   │   ├── GA_BaseAbility
│   │   ├── GA_BaseInteract
│   │   └── GA_BaseStaminaAbility
│   ├── Data/
│   ├── AEAbilitySystemComponent
│   ├── AS_BaseCombat
│   ├── AS_HealthSet
│   └── HealthComponent
├── Actor/
│   ├── Projectile/
│   ├── AEEnemySpawnPoint
│   └── AEQuestLocationActor
├── Characters/
│   ├── Enemy/
│   │   ├── Data/
│   │   └── BaseEnemyCharacter
│   ├── Player/
│   │   ├── Data/
│   │   ├── AEPlayerController
│   │   ├── AEWeaponComponent
│   │   ├── OcclusionFadeComponent
│   │   └── PlayerCharacter
│   ├── AEPlayerState
│   └── BaseCharacter
├── Core/
│   ├── AEGameMode
│   ├── AEGameplayTags
│   ├── AEGloabalHelper
│   ├── AEHUD
│   ├── AEInRaidGameMode
│   ├── AESaveGame
│   ├── GamePhaseSubsystem
│   ├── RaidSessionSubsystem
│   └── SaveGameSubsystem
├── FX/
├── Interaction/
│   └── InteractionComponent
├── Interfaces/
│   └── Interactable
├── Inventory/
│   ├── Data/
│   ├── LootSystem/
│   ├── Widgets/
│   ├── InventoryComponent
├── Quest/
│   └── AEQuestSubSystem
└── Widgets/
    ├── AEGameHUDWidget
    ├── StatBarContainerWidget
    └── StatBarWidget
```
