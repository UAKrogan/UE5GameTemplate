# GameFeature_SampleCharacter

`[TEMPLATE EXAMPLE]` — demonstrates a complete GAS-ready character setup via the Atlas pipeline. Content-only (no C++). Safe to delete in real projects.

## Assets to author in the editor (`Content/`)

| Asset | Type | Purpose |
|---|---|---|
| `GFD_SampleCharacter` | `UGameFeatureData` | Feature descriptor; lists the actions below |
| `Abilities/GA_Sample_Jump` | Blueprint of `UAtlasBaseGameplayAbility` | Sample jump ability |
| `Abilities/DA_Sample_AbilitySet` | `UAtlasAbilitySet` | Grants `GA_Sample_Jump` with input tag `Atlas.Input.Ability.Jump` |
| `Input/IA_Sample_Jump` | `UInputAction` | Jump input action |
| `Input/IMC_Sample_Character` | `UInputMappingContext` | Maps Space/Gamepad A to `IA_Sample_Jump` |
| `Input/DA_Sample_InputConfig` | `UAtlasInputConfigData` | Binds `IA_Sample_Jump` to `Atlas.Input.Ability.Jump` |
| `PawnData/DA_Sample_PawnData` | `UAtlasPawnData` | References the ability set and input config |

## Actions in `GFD_SampleCharacter`

- `Atlas: Add Abilities` — ActorClass: `AAtlasCharacter`, AbilitySets: `DA_Sample_AbilitySet`
- `Atlas: Add Input Config` — InputConfigs: `DA_Sample_InputConfig`

## How to test

1. Enable the feature via the Game Features editor UI (or `UGameFeaturesSubsystem::LoadAndActivateGameFeaturePlugin`).
2. PIE — verify the sample ability is granted (`showdebug abilitysystem`).
3. Press jump — verify activation. Deactivate the feature — verify the grant is revoked.
