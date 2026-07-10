# 01 — Target Architecture

## Guiding Principles

- The host `Game` module must remain a thin bootstrap. It assembles the project but contains no reusable logic.
- All reusable framework logic lives in dedicated modules that do not depend on `Game`.
- Feature content lives in Game Feature plugins, not in source modules.
- Module boundaries follow UE conventions: subsystems for global services, components for per-actor behavior, data assets for configuration.
- No circular dependencies. Dependency direction is strictly enforced.
- GAS, Enhanced Input, Common UI, and Modular Gameplay must be first-class citizens, not afterthoughts.

---

## Recommended Final Module Set

### Source Modules (always compiled into the project)

| Module | Purpose |
|---|---|
| `Game` | Thin host. Entry point only. Assembles other modules. |
| `GameUtils` | Logging, assertions, debug helpers. Zero game logic. |
| `GameCore` | Game instance, subsystems, runtime services, asset manager, game flow, level flow, save/load, settings. |
| `GameActors` | All actor/pawn/character/controller/state base classes. GAS-ready. Input-extension-ready. |
| `GameUI` | Common UI screen stack, layers, HUD, modals, notifications, loading UI, input mode management. |

Five modules is the correct number. Do **not** add `GameLoading`, `GameInput`, or `GameFeaturesSupport` as separate modules. Their responsibilities belong in existing modules per the boundaries below. Module proliferation increases compile complexity for marginal benefit.

### Optional Future Plugins (not source modules)

| Plugin | Purpose | When to add |
|---|---|---|
| `GameFeature_SampleCharacter` | Sample playable character using GAS, pawn data, input mapping | Phase 8 |
| `GameFeature_SampleUI` | Sample main menu, pause menu, HUD, settings screens | Phase 8 |
| `GameFeature_SampleWorld` | Sample gameplay map, game mode defaults | Phase 8 |

These must be Game Feature plugins so they can be enabled/disabled without changing source modules.

---

## Allowed Dependency Graph

```
┌─────────────────────────────────────────────────────────────────────┐
│  Game Feature Plugins (optional, depend on source modules only)     │
│  GameFeature_SampleCharacter / GameFeature_SampleUI / etc.          │
└──────────────────────────┬──────────────────────────────────────────┘
                           │ (depends on)
┌──────────────────────────▼──────────────────────────────────────────┐
│  Game (thin host)                                                   │
│  Deps: GameActors, GameCore, GameUI, GameUtils                      │
└───────┬─────────────┬────────────────┬───────────────┬─────────────┘
        │             │                │               │
┌───────▼───┐  ┌──────▼──────┐  ┌─────▼─────┐  ┌────▼──────────────┐
│ GameActors│  │  GameCore   │  │  GameUI   │  │    GameUtils       │
│           │  │             │  │           │  │                    │
│ Deps:     │  │ Deps:       │  │ Deps:     │  │ Deps:              │
│ GameUtils │  │ GameUtils   │  │ GameUtils │  │ Core, CoreUObject  │
│ GameCore* │  │ GAS modules │  │ UMG       │  │                    │
│ EnhInput  │  │ GameTags    │  │ CommonUI  │  │                    │
│ AIModule  │  │ GameTasks   │  │ Slate(p)  │  │                    │
└───────────┘  └─────────────┘  └───────────┘  └────────────────────┘
```

\* `GameActors` may need a **private** dependency on `GameCore` for GAS initialization hooks, or use forward declarations + interfaces only. The cleanest approach: `GameActors` adds `GameplayAbilities` to its own Build.cs and does **not** depend on `GameCore`. GAS initialization is coordinated through component contracts, not direct subsystem calls.

### Strict rules

1. `GameUtils` depends on nothing local.
2. `GameCore` depends on `GameUtils` only (plus engine GAS/Tags/Tasks modules).
3. `GameActors` depends on `GameUtils` only (plus engine GAS, EnhancedInput, AIModule). No dependency on `GameCore` or `GameUI`.
4. `GameUI` depends on `GameUtils` only. No dependency on `GameCore` or `GameActors`.
5. `Game` is the only module allowed to depend on all other modules.
6. Game Feature plugins may depend on any source module but **never** on another feature plugin.

---

## Module Responsibilities (Detailed)

### `Game`

**Must contain:**
- `UAtlasGameInstance` — entry point, subsystem resolution
- `IMPLEMENT_PRIMARY_GAME_MODULE`
- Module startup/shutdown logging

**Must not contain:**
- Any reusable logic
- Any actor class
- Any UI class
- Any system implementation

### `GameUtils`

**Must contain:**
- Log category declarations (`LogAtlas`, `LogAtlasActors`, `LogAtlasCore`, `LogAtlasUI`, `LogAtlasUtils`)
- Log macros (`ATLAS_LOG_*`, `ATLAS_LOG_DEBUG`)
- Assertion macros (`ATLAS_CHECK`, `ATLAS_ENSURE`, `ATLAS_ENSURE_MSG`)
- Scoped log helpers
- Generic low-level utilities with no game dependencies (string helpers, math helpers, type utilities)

**Must not contain:**
- Any UObject beyond utility objects
- Any gameplay or UI logic

### `GameCore`

**Must contain:**
- `UAtlasGameInstance` (currently in `Game` — keep it there; `GameCore` owns the subsystems it creates)
- `UAtlasGameInstanceSubsystem` — system registry, service locator
- `IAtlasSystem`, `FAtlasSystemsRegistry`
- Game flow subsystem (`UAtlasGameFlowSubsystem`) — startup state, menu state, gameplay state
- Level transition subsystem (`UAtlasLevelTransitionSubsystem`) — request, pre-travel, travel, post-load
- Loading screen service (interface + default implementation)
- `UAtlasAssetManager` — custom asset manager, primary asset types
- Gameplay tag initialization (`UAtlasGameplayTagManager` or tag data asset + `NativeGameplayTags`)
- `UAtlasDeveloperSettings` — project-level template settings
- Save/load pipeline (existing `FAtlasSaveSystem`, `FAtlasLoadSystem`, etc.)
- `UAtlasSavableComponent`, `IAtlasSavable`, save adapters
- Feature activation support (listening to `UGameFeaturesSubsystem`)

**Must not contain:**
- Any actor class
- Any widget class
- Any input handling
- Direct CommonUI includes

### `GameActors`

**Must contain:**
- `AAtlasCharacter`, `AAtlasPawn`, `AAtlasPlayerController`, `AAtlasAIController`, `AAtlasGameMode`, `AAtlasGameState`, `AAtlasPlayerState`
- `AAtlasVehiclePawn` — GAS-ready vehicle base
- `AAtlasMountPawn` — GAS-ready mount base
- `UAtlasPawnExtensionComponent` — coordinates GAS init, pawn data, input extension
- `UAtlasAbilityExtensionComponent` — owns ASC on pawn or delegates to player state
- `UAtlasInputExtensionComponent` — Enhanced Input wiring, ability input binding
- `UAtlasMovementExtensionComponent` — movement mode tracking, mount/vehicle transitions
- `UAtlasVehicleExtensionComponent` — enter/exit vehicle lifecycle
- `UAtlasMountExtensionComponent` — enter/exit mount lifecycle
- `UAtlasAbilitySystemComponent` — custom ASC (thin extension of `UAbilitySystemComponent`)
- `UAtlasBaseAttributeSet` — shared attribute set base
- `UAtlasPawnData` — data asset per pawn type: ability sets, input mapping contexts, camera config
- `UAtlasAbilitySet` — data asset: set of abilities + effects + attribute sets to grant

**Must not contain:**
- UI widgets
- Save/load logic (use save adapters via `IAtlasSavable`)
- Level transition logic

### `GameUI`

**Must contain:**
- `UAtlasUISubsystem` — screen stack, layer management, input mode switching
- `UAtlasActivatableWidget` — base Common UI activatable widget
- `UAtlasHUDLayer` — root HUD widget layer (always visible during gameplay)
- `UAtlasModalLayer` — modal/dialog layer
- `UAtlasNotificationLayer` — toast/notification layer
- `UAtlasLoadingLayer` — loading screen layer
- `UAtlasScreenDefinition` — data asset: widget class, layer, input mode, navigation config
- `UAtlasScreenRegistry` — data asset: all registered screens by ID
- `UAtlasHUDWidget` — base HUD widget
- `UAtlasMenuWidget` — base full-screen menu widget
- `UAtlasModalWidget` — base modal dialog widget
- `UAtlasNotificationWidget` — base notification/toast widget
- `UAtlasLoadingScreenWidget` — base loading screen widget
- Input mode management (Game, Menu, GameAndMenu, GamepadCursor)
- Controller glyph/input prompt routing (data-driven platform detection)

**Must not contain:**
- Gameplay logic
- Actor references (except through delegates/viewmodels)
- Save/load logic

---

## What Should Become a Plugin vs. Stay a Source Module

| Candidate | Recommendation | Reason |
|---|---|---|
| Game Features support | Keep in `GameCore` as a listener | Not enough standalone logic to warrant a plugin |
| Loading screen | Keep in `GameCore` (logic) + `GameUI` (widget) | Tightly coupled to level flow |
| Sample content | Game Feature plugins only | Must be optional and disabled for production use |
| Asset manager | Keep in `GameCore` | One per project |
| `RiderLink` | Developer-only, already local | Correct, keep excluded from version control |

Do not extract core systems into plugins unless they are truly optional. Plugins add complexity (loading phase, plugin descriptor, dependency declaration overhead) that is only justified when the feature is truly optional or shared across projects.
