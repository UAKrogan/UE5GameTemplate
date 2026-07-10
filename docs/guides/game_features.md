# Game Feature Plugins

Optional, activatable bundles of gameplay content. All real gameplay should live here; the source modules stay game-agnostic.

## Dependency direction (enforced)

```
Feature plugins  ──depends on──►  Source modules (GameActors, GameCore, GameUI, GameUtils)
```

Never the reverse: source modules must never include feature plugin headers or reference their assets. Feature plugins must not depend on each other.

## Creating a feature plugin

1. Create `Plugins/GameFeatures/GameFeature_<Name>/GameFeature_<Name>.uplugin`:
   - `"ExplicitlyLoaded": true`, `"BuiltInInitialFeatureState": "Registered"` (or `"Active"` to auto-activate)
   - `"CanContainContent": true`; list plugin dependencies (`GameFeatures`, `ModularGameplay`, ...)
2. In the editor, create a `UGameFeatureData` asset at the plugin content root (`GFD_<Name>`).
3. Add actions to the feature data (below).
4. Activate/deactivate from the editor's Game Features panel, or at runtime via `UGameFeaturesSubsystem::LoadAndActivateGameFeaturePlugin`.

The two `[TEMPLATE EXAMPLE]` plugins (`GameFeature_SampleCharacter`, `GameFeature_SampleUI`) document a full asset checklist in their READMEs.

## Available actions

**Engine built-ins:**

| Action | Purpose |
|---|---|
| `UGameFeatureAction_AddComponents` | Add components to actor classes at runtime (uses the receiver pattern all Atlas base actors implement) |
| `UGameFeatureAction_AddGameplayCuePath` | Register gameplay cue paths |

**Custom Atlas actions:**

| Action | Module | Purpose |
|---|---|---|
| `Atlas: Add Abilities` | `GameActors` | Grants ability sets to pawns of a class while active |
| `Atlas: Add Input Config` | `GameActors` | Applies input configs to local Atlas player controllers |
| `Atlas: Add Screens` | `GameUI` | Registers screen definitions with the UI subsystem |
| `Atlas: Add HUD Elements` | `GameUI` | Injects widgets into named HUD slots |

## How the custom actions behave

**Add Abilities** — registers a Modular Gameplay extension handler for the configured pawn class, so both live and late-spawning pawns are covered. If a pawn's ability system isn't initialized yet (not possessed), the grant defers until `OnAbilitySystemInitialized`. Deactivation revokes every granted set.

**Add Input Config** — applies each `UAtlasInputConfigData` (mapping context + ability bindings) through the controller's input extension component; removal on deactivation takes the mapping contexts back out.

**Add Screens** — calls `UAtlasUISubsystem::RegisterScreen` for each entry; deactivation pops the screen if it is on-screen, then unregisters it. Runtime registrations take precedence over the authored registry.

**Add HUD Elements** — adds widgets to the active `UAtlasHUDWidget`'s named slots; also listens for HUD screens pushed later. Deactivation removes the slots it filled.

## Gameplay tags from plugins

Declare native tags in the plugin's own module following the `Atlas.Feature.<Name>.*` convention, or add a tag ini under the plugin `Config/Tags/`. Never add feature tags to the base modules.

## Savable actors from plugins

Actors added by a feature participate in save/load automatically once they carry a `UAtlasSavableComponent` — the save collector discovers them by component, not by class list.
