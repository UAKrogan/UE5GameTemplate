# 08 — Game Features and Modular Gameplay Architecture

---

## Required Plugins

Enable in `Game.uproject`:

```json
{
    "Name": "GameFeatures",
    "Enabled": true
},
{
    "Name": "ModularGameplay",
    "Enabled": true
}
```

After enabling, run `Generate Project Files` to update the solution.

---

## What Game Features Enable

- Discrete, optionally-loadable bundles of gameplay content
- Adding abilities, input mappings, UI screens, HUD elements, actor components, gameplay tags, and pawn data **without modifying base source modules**
- Clean separation between framework code and game content
- Multiple games sharing the same framework base
- Sample content that can be stripped without affecting the template

---

## Project Plugin Structure

```
Plugins/
├── Developer/
│   └── RiderLink/          (already present, dev-only)
└── GameFeatures/
    ├── GameFeature_SampleCharacter/
    │   ├── GameFeature_SampleCharacter.uplugin
    │   ├── Config/
    │   ├── Content/
    │   └── Source/
    │       └── GameFeature_SampleCharacter/
    ├── GameFeature_SampleUI/
    └── GameFeature_SampleWorld/
```

Each feature plugin must:
- Depend on base source modules (`GameActors`, `GameCore`, `GameUI`, `GameUtils`)
- Never depend on another feature plugin
- Declare `"bCanBeUsedWithUnrealHeaderTool": true` in `.uplugin` if it has C++ source

---

## `UGameFeatureData` Usage

Each feature plugin has one `UGameFeatureData` asset (in `Content/`) that lists all actions to run when the feature is activated:

```
GameFeature_SampleCharacter/Content/GFD_SampleCharacter.uasset (UGameFeatureData)
Actions:
  - UGameFeatureAction_AddComponents
  - UAtlasGameFeatureAction_AddAbilities
  - UAtlasGameFeatureAction_AddInputConfig
  - UAtlasGameFeatureAction_AddPawnData
```

---

## Feature Actions

### UE built-in actions to use:

| Action | Purpose |
|---|---|
| `UGameFeatureAction_AddComponents` | Add `UActorComponent` subclasses to existing actor classes at runtime |
| `UGameFeatureAction_AddGameplayCuePath` | Register gameplay cue paths |

### Custom Atlas actions to implement:

| Action Class | Module | Purpose |
|---|---|---|
| `UAtlasGameFeatureAction_AddAbilities` | `GameActors` | Grant ability sets to actors matching a tag or class filter |
| `UAtlasGameFeatureAction_AddInputConfig` | `GameActors` | Register additional `FAtlasInputConfig` for a pawn type |
| `UAtlasGameFeatureAction_AddScreens` | `GameUI` | Register additional screen definitions to `UAtlasScreenRegistry` |
| `UAtlasGameFeatureAction_AddHUDElements` | `GameUI` | Inject HUD element widgets into the active HUD layer |
| `UAtlasGameFeatureAction_AddPawnData` | `GameActors` | Register an alternative `UAtlasPawnData` for a specific game mode or context |

Each action implements:
- `OnGameFeatureActivating(FGameFeatureActivatingContext&)` — register/apply content
- `OnGameFeatureDeactivating(FGameFeatureDeactivatingContext&)` — unregister/remove content

---

## Adding Abilities via Feature Plugin

```
UAtlasGameFeatureAction_AddAbilities
  ActorClass: AAtlasCharacter   (or a tag filter)
  AbilitySet: DA_SampleCharacter_AbilitySet
```

When the feature activates:
1. Action finds all live actors matching the filter
2. Calls `PawnExtComp->AbilityExtComp->GrantAbilitySet(AbilitySet, ASC)`
3. Stores a grant handle for revocation on deactivation

When the feature deactivates:
1. Action calls `RevokeAbilitySet(Handle)` on each live actor

Actors spawned **after** activation: handled via `UGameFrameworkComponentManager::AddReceiver()` so the action's grants are applied to late-spawning actors.

---

## Adding Input Mappings via Feature Plugin

```
UAtlasGameFeatureAction_AddInputConfig
  ActorClass: AAtlasPlayerController
  InputConfig: DA_SampleCharacter_InputConfig
```

When activating:
1. Finds all live `AAtlasPlayerController` instances
2. Calls `InputExtComp->ApplyInputMappingContexts(Config)`
3. Stores config reference for removal on deactivation

---

## Adding UI Screens via Feature Plugin

```
UAtlasGameFeatureAction_AddScreens
  Screens:
    - ScreenId: "SampleInventory"
      Definition: DA_SampleInventoryScreen
```

When activating:
1. Action calls `UAtlasUISubsystem::RegisterScreen(ScreenId, Definition)`

When deactivating:
1. Action calls `UAtlasUISubsystem::UnregisterScreen(ScreenId)`
2. If the screen is active, it is first popped

---

## Adding HUD Elements via Feature Plugin

```
UAtlasGameFeatureAction_AddHUDElements
  Elements:
    - SlotName: "HealthBar"
      WidgetClass: WBP_SampleHealthBar
```

When activating:
1. Finds active `UAtlasHUDWidget` instance
2. Calls `HUDWidget->AddHUDElement("HealthBar", WidgetClass)`

When deactivating:
1. Calls `HUDWidget->RemoveHUDElement("HealthBar")`

---

## Adding Gameplay Tags via Feature Plugin

Gameplay tags added by feature plugins must be declared in:
```
GameFeature_SampleCharacter/Config/Tags/SampleCharacter.ini
```

And referenced in the `.uplugin`:
```json
"AdditionalFieldsToSerialize": [
    "GameplayTagList"
]
```

Or use `UGameplayTagsManager::Get().AddNativeGameplayTag()` in the plugin module's startup.

---

## Dependency Direction (Enforced)

```
Base source modules (GameActors, GameCore, GameUI, GameUtils)
        ▲
        │ depends on
Feature plugins (GameFeature_SampleCharacter, etc.)
```

**Never the reverse.** Source modules must never `#include` from feature plugin headers or reference feature plugin assets directly.

---

## Activation and Deactivation Lifecycle

1. **Startup activation:** `UAtlasDeveloperSettings::bEnableGameFeatures` — if true, activate all enabled feature plugins automatically at game start via `UGameFeaturesSubsystem`
2. **Manual activation:** `UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL, Callback)`
3. **Deactivation:** `UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL, Callback)`
4. **Game Feature state observer:** `UAtlasFeaturePolicy` (in `GameCore`) observes state changes to coordinate with runtime systems

---

## Sample Feature Plugin: `GameFeature_SampleCharacter`

**Purpose:** Demonstrates a complete GAS-ready character setup.

**Contents (template examples, not real gameplay):**
```
Content/
  Abilities/
    DA_Sample_AbilitySet.uasset          (UAtlasAbilitySet)
    GA_Sample_Jump.uasset                (UAtlasBaseGameplayAbility)
    GA_Sample_Sprint.uasset              (UAtlasBaseGameplayAbility)
  Input/
    IA_Sample_Jump.uasset                (UInputAction)
    IA_Sample_Sprint.uasset              (UInputAction)
    IMC_Sample_Character.uasset          (UInputMappingContext)
    DA_Sample_InputConfig.uasset         (FAtlasInputConfig)
  PawnData/
    DA_Sample_PawnData.uasset            (UAtlasPawnData)
  GFD_SampleCharacter.uasset            (UGameFeatureData)
```

Clearly marked in editor with `[TEMPLATE EXAMPLE]` prefix on asset names.

---

## `UGameFrameworkComponentManager` Integration

Atlas base actors must call:
```cpp
// In AAtlasCharacter::BeginPlay():
UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

// In AAtlasCharacter::EndPlay():
UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
```

This allows `UGameFeatureAction_AddComponents` to retroactively add components to actors that were spawned before a feature activated.

The same pattern applies to `AAtlasPlayerController`, `AAtlasPlayerState`, `AAtlasGameState`, `AAtlasGameMode`.
