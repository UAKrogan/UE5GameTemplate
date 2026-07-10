# Modules

Per-module responsibilities, boundaries, and the checklist for adding a new module.

---

## `Game` — thin host

**Must contain:**
- `UAtlasGameInstance` — entry point, subsystem resolution
- `IMPLEMENT_PRIMARY_GAME_MODULE`
- Module startup/shutdown logging

**Must not contain:** any reusable logic, actor classes, UI classes, or system implementations. If you are adding logic here, it belongs in another module.

## `GameUtils` — foundation

**Must contain:**
- Log category declarations (`LogAtlas`, `LogAtlasActors`, `LogAtlasCore`, `LogAtlasUI`, `LogAtlasUtils`)
- Log macros (`ATLAS_LOG_*`, `ATLAS_LOG_DEBUG`)
- Assertion macros (`ATLAS_CHECK`, `ATLAS_ENSURE`, `ATLAS_ENSURE_MSG`)
- Generic low-level utilities with no game dependencies

**Must not contain:** gameplay or UI logic, or UObjects beyond utility objects.

## `GameCore` — runtime services

**Must contain:**
- `UAtlasGameInstanceSubsystem` — system registry, service locator
- `IAtlasSystem`, `FAtlasSystemsRegistry`
- Game flow / level transition / loading screen subsystems, `AAtlasPostLoadTrigger`
- `UAtlasAssetManager`, `UAtlasDeveloperSettings`, native gameplay tag declarations
- Save/load pipeline (`FAtlasSaveSystem`, `FAtlasLoadSystem`, scheduler, autosave, serialization, migrations, slot manifest)
- `UAtlasSavableComponent`, `IAtlasSavable`, save adapters

**Must not contain:** gameplay actor classes (the post-load trigger is the deliberate exception — it exists solely to call the transition subsystem), widget classes, input handling, or direct CommonUI includes. The loading screen subsystem handles its widget generically as `UUserWidget` via a soft class path.

## `GameActors` — actor foundation

**Must contain:**
- `AAtlasCharacter`, `AAtlasPawn`, `AAtlasPlayerController`, `AAtlasAIController`, `AAtlasGameMode`, `AAtlasGameState`, `AAtlasPlayerState`
- Extension components (`UAtlasPawnExtensionComponent`, `UAtlasAbilityExtensionComponent`, `UAtlasInputExtensionComponent`, `UAtlasMovementExtensionComponent`)
- `UAtlasAbilitySystemComponent`, `UAtlasBaseAttributeSet`, `UAtlasBaseGameplayAbility`
- Data asset types for pawn configuration (`UAtlasPawnData`, `UAtlasAbilitySet`, `UAtlasInputConfigData`)
- Game Feature actions targeting actors (`Atlas: Add Abilities`, `Atlas: Add Input Config`)

**Must not contain:** UI widgets, save/load logic (participate via `IAtlasSavable` adapters instead), or level transition logic.

**Dependency note:** `GameActors` depends on the engine GAS modules directly (`GameplayAbilities`, `GameplayTags`, `GameplayTasks`, plus `ModularGameplay`/`GameFeatures` for the receiver pattern and actions) and must never depend on `GameCore` or `GameUI`.

## `GameUI` — Common UI layer

**Must contain:**
- `UAtlasUISubsystem` — screen stack, layer management, input mode switching
- Base widget classes (activatable, HUD, menu, modal, notification, loading screen)
- Screen definition/registry data assets, `UAtlasUIDeveloperSettings`
- Input mode management and controller glyph routing (`UAtlasGlyphSubsystem`, `UAtlasInputGlyphData`)
- Game Feature actions targeting UI (`Atlas: Add Screens`, `Atlas: Add HUD Elements`)

**Must not contain:** gameplay logic, actor references (except through delegates/viewmodels), or save/load logic.

**Dependency note:** UI configuration lives here (not in GameCore's settings) because `GameUI` must not depend on `GameCore` and vice versa.

---

## Enabled Engine Plugins

| Plugin | Why |
|---|---|
| `GameplayAbilities` | GAS: ability system components, attributes, effects |
| `GameFeatures` | Optional feature plugins activated at runtime |
| `ModularGameplay` | `UGameFrameworkComponentManager` receiver pattern on base actors |
| `CommonUI` | Activatable widget stacks, input routing, Common Input device detection |

---

## Adding a New Module (checklist)

1. Create `Source/<Name>/<Name>.Build.cs`:
   - `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs`
   - Explicit `PublicIncludePaths` / `PrivateIncludePaths` (`Public`, `Private`)
   - Declare dependencies that respect the graph in [architecture.md](architecture.md)
2. Create the module implementation:
   - `Public/<Name>.h` + `Private/<Name>.cpp` with `IMPLEMENT_MODULE(FDefaultModuleImpl, <Name>)`
3. Add the module to `Game.uproject` under `"Modules"` with the appropriate `Type` and `LoadingPhase`.
4. Add the module to `GameTarget.cs` / `GameEditorTarget.cs` if it must be linked into the targets explicitly.
5. Add a log category in `GameUtils` if the module needs its own logging macro.
6. Regenerate project files and build.

Think twice before adding a module — see "Five source modules, no more" in [architecture.md](architecture.md).
