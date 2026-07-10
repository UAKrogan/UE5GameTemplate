# Architecture

High-level structure of the Atlas framework: modules, runtime lifecycle, and the design decisions behind them.

---

## Module Dependency Graph

Dependencies flow strictly one way. A module may only depend on modules to its right (plus listed engine modules).

```
┌─────────────────────────────────────────────────────────────────────┐
│  Game Feature Plugins (optional, depend on source modules only)     │
└──────────────────────────┬──────────────────────────────────────────┘
                           │ (depends on)
┌──────────────────────────▼──────────────────────────────────────────┐
│  Game (thin host)                                                   │
│  Deps: GameActors, GameCore, GameUI, GameUtils                      │
└───────┬─────────────┬────────────────┬───────────────┬─────────────┘
        │             │                │               │
┌───────▼───┐  ┌──────▼──────┐  ┌─────▼─────┐  ┌──────▼────────────┐
│ GameActors│  │  GameCore   │  │  GameUI   │  │    GameUtils      │
│           │  │             │  │           │  │                   │
│ Deps:     │  │ Deps:       │  │ Deps:     │  │ Deps:             │
│ GameUtils │  │ GameUtils   │  │ GameUtils │  │ Core, CoreUObject │
│ GAS mods  │  │ GAS modules │  │ UMG       │  │                   │
│ EnhInput  │  │ DevSettings │  │ CommonUI  │  │                   │
│ AIModule  │  │ (UMG, Json  │  │ CommonInp │  │                   │
│ ModularGP │  │  private)   │  │ GameFeats │  │                   │
│ GameFeats │  │             │  │ DevSett.  │  │                   │
└───────────┘  └─────────────┘  └───────────┘  └───────────────────┘
```

### Strict rules

1. `GameUtils` depends on nothing local.
2. `GameCore` depends on `GameUtils` only (plus engine `GameplayAbilities`, `GameplayTags`, `GameplayTasks`, `DeveloperSettings`; private `UMG`/`Slate`/`SlateCore` for the generic loading widget and `Json` for the save manifest).
3. `GameActors` depends on `GameUtils` only (plus engine GAS modules, `EnhancedInput`, `AIModule`, `ModularGameplay`, `GameFeatures`). **No dependency on `GameCore` or `GameUI`.**
4. `GameUI` depends on `GameUtils` only (plus `UMG`, `CommonUI`, `CommonInput`, `InputCore`, `DeveloperSettings`, `GameFeatures`; private `Slate`/`SlateCore`). **No dependency on `GameCore` or `GameActors`.**
5. `Game` is the only source module allowed to depend on all other modules.
6. Game Feature plugins may depend on any source module but never on another feature plugin.

When a design conflicts with this graph, the graph wins — bridge with soft object paths, delegates, or data assets. Existing examples: UI configuration lives in `UAtlasUIDeveloperSettings` (GameUI) rather than GameCore's settings; GameCore's loading screen subsystem handles its widget generically as `UUserWidget` via a soft class path; `AAtlasPostLoadTrigger` lives in GameCore because it exists solely to call the transition subsystem.

## Runtime Lifecycle

```
UAtlasGameInstance::Init()
        │
        ▼
UAtlasGameInstanceSubsystem::Initialize()
        │
        ├── RegisterSystems()        (FAtlasSystemsRegistry::Register)
        │       └── RegisterSystem(MakeShared<FAtlasSaveSystem>()) ...
        │
        └── InitializeSystems()      (in registration order)

UAtlasGameInstanceSubsystem::Deinitialize()
        └── ShutdownSystems()        (reverse registration order)
```

Systems are pure C++ (`IAtlasSystem`, `TSharedPtr`-owned, not `UObject`s), stored in a `TMap<FName, TSharedPtr<IAtlasSystem>>`, and resolved by interface via `GetSystem<T>()` / `SupportsInterface(T::InterfaceName())`.

Engine subsystems (`UGameInstanceSubsystem` subclasses) are used for anything that must interact with UObject lifecycles, world callbacks, or Blueprint: UI, game flow, level transitions.

## Layering

```
GameUtils        ← foundation: logging, assertions, helpers
    ▲
GameCore / GameActors / GameUI     ← parallel framework pillars (no cross-deps)
    ▲
Game             ← thin host that assembles the pillars
    ▲
Game Feature plugins   ← optional content and gameplay features
```

## Key Design Decisions

**Five source modules, no more.** `GameLoading`, `GameInput`, or `GameFeaturesSupport` are intentionally *not* separate modules — their responsibilities live in `GameCore`, `GameActors`, and `GameUI` respectively. Module proliferation increases compile and dependency complexity for marginal benefit.

**`GameActors` does not depend on `GameCore`.** GAS initialization is coordinated through component contracts (extension components on the actors) rather than direct subsystem calls. `GameActors` gets GAS from the engine modules directly.

**Pure C++ systems for services, subsystems for engine integration.** Save/load and similar services are `IAtlasSystem` implementations — cheap, testable, no UObject overhead. Anything that needs engine lifecycle hooks is a `UGameInstanceSubsystem`.

**Feature content lives in Game Feature plugins.** Source modules provide reusable framework only; sample characters, UI screens, and maps are shipped as optional `GameFeature_*` plugins that can be enabled/disabled without touching source. The `GameFeatures` and `ModularGameplay` engine plugins are enabled project-wide to support this.

**Data-driven configuration.** Pawn setup (`UAtlasPawnData`), ability grants (`UAtlasAbilitySet`), screens (`UAtlasScreenRegistry`), and input (`UAtlasInputConfigData`) are data assets resolved through the asset manager, so games built on the template configure behavior without subclassing.
