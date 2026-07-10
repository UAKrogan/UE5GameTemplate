# 00 — Current Repository Analysis

## Project Configuration

| Property | Value |
|---|---|
| Engine | Unreal Engine 5.7 |
| Project descriptor | `Game.uproject` |
| Default game instance | `UAtlasGameInstance` (`/Script/Game.AtlasGameInstance`) |
| Default startup map | `/Engine/Maps/Templates/OpenWorld` (engine template, not a project map) |
| Input system | `EnhancedPlayerInput` + `EnhancedInputComponent` |
| Graphics | DX12, SM6, Lumen, VSM, Substrate, Ray Tracing (desktop/maximum) |
| Runtime targets | `GameTarget`, `GameEditorTarget` — both `BuildSettingsVersion.V6`, `Unreal5_7` |
| UI framework declared | `CommonUI` (plugin dependency in `GameUI`) |

---

## Modules

### `Game` — primary host module

- Purpose: bootstrap entry point only
- Key class: `UAtlasGameInstance` (extends `UGameInstance`)
- `IMPLEMENT_PRIMARY_GAME_MODULE`
- `Init()` resolves `UAtlasGameInstanceSubsystem`
- `Shutdown()` logs shutdown
- Build.cs public deps: `Core, CoreUObject, Engine, GameActors, GameCore, GameUI, GameUtils`
- **Correctly thin.** No gameplay logic.

### `GameActors` — gameplay framework base classes

- `AAtlasCharacter` extends `ACharacter` — constructor + `BeginPlay` + `InitializeCharacter()` virtual
- `AAtlasPawn` extends `APawn` — constructor + `BeginPlay` + `InitializePawn()` virtual
- `AAtlasPlayerController` extends `APlayerController` — `BeginPlay`, `SetupInputComponent`, `InitializeController()`, `InitializeInput()` virtuals
- `AAtlasAIController` extends `AAIController` — `BeginPlay` + `InitializeAI()` virtual
- `AAtlasGameMode` extends `AGameModeBase` — `StartPlay` + `InitializeGameMode()` virtual
- `AAtlasGameState` extends `AGameStateBase` — `BeginPlay` + `InitializeGameState()` virtual
- `AAtlasPlayerState` extends `APlayerState` — `BeginPlay` + `InitializePlayerState()` virtual
- Build.cs public deps: `Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, GameUtils`
- **No GAS classes, no ASC, no pawn data, no Enhanced Input bindings beyond stub override.**

### `GameCore` — runtime systems

- `IAtlasSystem` — pure C++ base interface with `Initialize`, `Shutdown`, `GetSystemName`, `SupportsInterface`
- `UAtlasGameInstanceSubsystem` — central orchestrator, owns systems in `TMap<FName, TSharedPtr<IAtlasSystem>>` + ordered array
- `FAtlasSystemsRegistry` — static `Register()` that creates and pushes systems into the subsystem
- `IAtlasSaveSystem` / `FAtlasSaveSystem` — full save pipeline
- `IAtlasLoadSystem` / `FAtlasLoadSystem` — full load pipeline
- `FAtlasSaveScheduler` — priority queue (Manual > Event > Autosave), debounce, timer tick
- `FAtlasAutosaveManager` — 5-slot ring buffer (Autosave_0..4), index persisted to `AutosaveIndex.sav`
- `FAtlasBinaryWriter` / `FAtlasBinaryReader` — versioned chunked binary serialization
- `FAtlasSaveCollector` — walks the world for `UAtlasSavableComponent` actors, builds `FAtlasWorldSnapshot`
- `FAtlasFileStorage` — raw binary read/write to `Saved/SaveGames/`
- `FAtlasSaveGameSnapshot` — flat key/value store (int, float, string, vector, rotator, transform) + metadata
- `FAtlasWorldSnapshot` / `FAtlasActorSnapshot` / `FAtlasDataChunk` — actor-level save data types
- `FAtlasSaveContext` / `FAtlasLoadContext` — archive-backed chunked context for `IAtlasSavable`
- `IAtlasSavable` / `UAtlasSavable` — UE interface pair for actor/component opt-in
- `UAtlasSavableComponent` — gives actors a stable `FGuid ActorId`
- `UAtlasGASSaveAdapter` — `UActorComponent` + `IAtlasSavable`, captures/restores ASC attributes and active effects
- `UAtlasInventorySaveAdapter` — `UActorComponent` + `IAtlasSavable`, generic item array, no real inventory logic
- `UAtlasSaveGame` / `FAtlasSaveGameMetadata` — UE `USaveGame` wrapper for metadata
- Build.cs public deps: `Core, CoreUObject, Engine, GameUtils, GameplayAbilities, GameplayTags, GameplayTasks`
- **GAS modules declared but zero GAS classes implemented.**

### `GameUI` — UI subsystem scaffold

- `UAtlasUISubsystem` extends `UGameInstanceSubsystem` — stub `OpenScreen(FName)` and `CloseScreen()` (log-only)
- Build.cs public deps: `Core, CoreUObject, Engine, UMG, CommonUI, GameUtils` / private: `Slate, SlateCore`
- **No widget classes, no screen stack, no layer management, no input mode switching.**

### `GameUtils` — shared logging layer

- `LogAtlas`, `LogAtlasActors`, `LogAtlasCore`, `LogAtlasUI`, `LogAtlasUtils` log categories
- `ATLAS_LOG`, `ATLAS_LOG_ACTORS`, `ATLAS_LOG_CORE`, `ATLAS_LOG_UI`, `ATLAS_LOG_UTILS` macros
- `ATLAS_CHECK`, `ATLAS_ENSURE`, `ATLAS_ENSURE_MSG` assertion macros
- `ATLAS_LOG_DEBUG` — stripped in Shipping
- `UAtlasScopedLog` — scoped timing/scope logger

---

## Plugins

| Plugin | State |
|---|---|
| `ModelingToolsEditorMode` | Enabled, editor-only |
| `Plugins/Developer/RiderLink` | Local developer plugin — must stay out of version control |

**No `GameFeatures` plugin. No `ModularGameplay` plugin. No `CommonGame` or Lyra-style plugins enabled.**

---

## Existing Build.cs Dependency Map

```
GameUtils    → Core, CoreUObject
GameCore     → GameUtils, GameplayAbilities, GameplayTags, GameplayTasks
GameActors   → GameUtils, InputCore, EnhancedInput, AIModule
GameUI       → GameUtils, UMG, CommonUI / private: Slate, SlateCore
Game         → GameActors, GameCore, GameUI, GameUtils
```

No circular dependencies exist today. The graph is clean.

---

## Missing or Incomplete Areas

### Critical gaps (blocking further phases)

| Area | Status |
|---|---|
| GAS — Ability System Component | Declared in Build.cs but zero implementation |
| GAS — base ability class | Missing |
| GAS — base attribute set | Missing |
| GAS — pawn data asset | Missing |
| GAS — initialization flow | Missing |
| Enhanced Input — input actions / mapping contexts | Missing |
| Enhanced Input — ability input binding | Missing |
| Pawn extension component | Missing |
| Input extension component | Missing |
| Game Features / Modular Gameplay plugins | Not enabled |
| Level transition subsystem | Missing |
| Loading screen subsystem | Missing |
| Startup flow | Missing |
| Main menu to gameplay flow | Missing |
| Common UI screen stack | Missing |
| UI layers (HUD, modal, loading) | Missing |
| Custom asset manager | Missing |
| Gameplay tag hierarchy / initialization | Missing |
| Developer settings (`UDeveloperSettings`) | Missing |
| Vehicle actor base class | Missing |
| Mount actor base class | Missing |
| Movement extension components | Missing |
| Project-owned startup map | Missing (using engine template map) |
| Sample Game Feature plugins | Missing |

### Incomplete areas (exist but thin)

| Area | Status |
|---|---|
| `AAtlasCharacter` | Has `InitializeCharacter()` hook, no GAS, no input, no movement config |
| `AAtlasPlayerController` | Has `InitializeInput()` hook stub, no actual Enhanced Input wiring |
| `UAtlasUISubsystem` | Has `OpenScreen`/`CloseScreen` stubs, no actual screen management |
| `FAtlasSaveSystem` | Implemented save pipeline but no actual async threading (uses async tasks) |

---

## Architectural Risks

### Risk 1 — `GameCore` is over-loaded
`GameCore` currently owns: runtime systems registry, save/load pipeline, GAS adapters, serialization, file I/O, world snapshot, savable interfaces. As GAS, level transition, asset management, and game flow get added, this module will become unmaintainably large.
**Recommendation:** Split systems into logical sub-modules or use subdirectory discipline strictly.

### Risk 2 — No startup flow guard
`UAtlasGameInstance::Init()` simply resolves the subsystem and logs. There is no startup map, no startup flow state machine, no splash/loading screen. Any UI or gameplay system added later will need to be retrofitted around this.

### Risk 3 — No Game Features / Modular Gameplay support
Both plugins are absent. All GAS wiring currently planned for `GameCore` will need to be moved/restructured once `ModularGameplay` is enabled. Adding it later without an extension component pattern will require significant refactoring.

### Risk 4 — `AAtlasGameMode` extends `AGameModeBase` not `AGameMode`
`AGameModeBase` does not support round-based play, match state, or the full GAS-compatible `AGameMode`. If the template ever needs match state (e.g. for multiplayer), this must be changed.
**Low risk today, but worth noting.**

### Risk 5 — Flat `FAtlasSaveGameSnapshot` conflicts with world snapshot
Two parallel save representations exist: the flat key/value `FAtlasSaveGameSnapshot` and the actor-level `FAtlasWorldSnapshot`. These serve different use cases but their relationship is not formalized in the save/load interfaces. This must be clarified before the save system grows.

### Risk 6 — `FAtlasSaveSystem` holds reference to `UAtlasGameInstanceSubsystem*` as raw pointer
`OwningSubsystem = nullptr` stored raw. The subsystem outlives the system (correct ordering), but this is fragile if shutdown order ever changes.

---

## Dependency Issues

- None in the current graph, but `GameCore` depending on `GameplayAbilities` while `GameActors` does not introduces a situation where actor classes cannot directly reference GAS types without also depending on `GameCore`. This will need to be resolved when GAS actor integration begins. The recommended solution is to move GAS actor dependencies into `GameActors` Build.cs (add `GameplayAbilities`) or use forward declarations only in interfaces.

---

## Naming Consistency

- `Atlas` prefix: used **consistently** across all classes, interfaces, structs, and macros.
- `F` prefix for pure C++ structs: correct.
- `I` prefix for interfaces: correct (`IAtlasSystem`, `IAtlasSavable`, `IAtlasSaveSystem`, `IAtlasLoadSystem`).
- `U` prefix for UObjects: correct (`UAtlasGameInstanceSubsystem`, `UAtlasSavableComponent`).
- `A` prefix for Actors: correct.
- Module log categories follow `LogAtlas*` convention correctly.
- `Initialize*()` hook naming is consistent across all actors.
- **No naming issues found.**

---

## Compile-Safety Concerns

- `UAtlasGASSaveAdapter` includes `GameplayTagContainer.h` and references `UAbilitySystemComponent*`, `UAttributeSet*`, `UGameplayEffect*`. These are only available because `GameCore` declares `GameplayAbilities` in its public deps. If `GameCore` ever splits, these deps must move with the adapter.
- `FAtlasLoadSystem` stores `bool bShuttingDown` and `bool bLoadInProgress` but these are not atomic. If async callbacks ever reach non-game threads, this will be a race condition.
- `FAtlasSaveSystem::OwningSubsystem` is a raw `UAtlasGameInstanceSubsystem*`. Safe today because lifecycle is controlled, but should be documented as a design constraint.
