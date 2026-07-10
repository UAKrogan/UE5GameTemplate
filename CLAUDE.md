# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Identity

**UE5GameTemplate** is an Unreal Engine 5.7 C++ multi-module framework (`Atlas` prefix) intended as a reusable base for different games. It is not a game itself. Never add game-specific mechanics (combat, inventory, quests, economy, etc.) unless explicitly asked for an isolated example.

Engine: `5.7` | Project descriptor: `Game.uproject` | Targets: `GameTarget`, `GameEditorTarget`

## Build Commands

All builds go through UnrealBuildTool. There is no standalone CLI build script; use the IDE or UBT directly.

**Generate project files (run from repo root):**
```
# Via UnrealVersionSelector (right-click Game.uproject > "Generate Visual Studio project files")
# Or via Rider: open Game.uproject directly
```

**Build from command line (UBT):**
```
# Editor build
"<UE5_ROOT>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" GameEditor Win64 Development "D:\Projects\UnrealEngine\UE5GameTemplate\Game.uproject"

# Game build
"<UE5_ROOT>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" Game Win64 Development "D:\Projects\UnrealEngine\UE5GameTemplate\Game.uproject"
```

There are no automated tests in the project yet. When tests are added they should use Unreal's Automation framework (`FAutomationTestBase`).

## Module Architecture

The allowed dependency graph is strictly one-way — modules may only depend on those to their right:

```
Game → GameActors, GameCore, GameUI, GameUtils
GameActors → GameUtils
GameCore → GameUtils  (+ GameplayAbilities, GameplayTags, GameplayTasks)
GameUI → GameUtils  (+ UMG, CommonUI, Slate, SlateCore)
GameUtils → (no local deps)
```

**Never create cross-module dependencies that go against this graph.** Feature plugins and the `Game` host module are the only places that may depend on multiple local modules simultaneously.

### Module Responsibilities

| Module | Purpose |
|---|---|
| `Game` | Thin host. `UAtlasGameInstance` bootstrap only. `IMPLEMENT_PRIMARY_GAME_MODULE`. |
| `GameActors` | GAS-ready base classes: `AAtlasCharacter`, `AAtlasPawn`, `AAtlasPlayerController`, `AAtlasAIController`, `AAtlasGameMode`, `AAtlasGameState`, `AAtlasPlayerState`. Provides overridable `Initialize*()` hooks, no gameplay logic yet. |
| `GameCore` | Runtime systems, subsystem orchestration, save/load pipeline, serialization, GAS adapters. |
| `GameUI` | `UAtlasUISubsystem` (`UGameInstanceSubsystem`) for screen flow. No widget assets yet. |
| `GameUtils` | Log categories, logging macros, assertion helpers. Nothing else. |

## Runtime Lifecycle

`UAtlasGameInstance::Init()` → resolves `UAtlasGameInstanceSubsystem`

`UAtlasGameInstanceSubsystem` (in `GameCore`) is the central orchestrator:
1. `Initialize()` → `RegisterSystems()` (delegates to `FAtlasSystemsRegistry`) → `InitializeSystems()`
2. `Deinitialize()` → `ShutdownSystems()` (reverse registration order)

Systems are stored as `TMap<FName, TSharedPtr<IAtlasSystem>>` and looked up by interface via `GetSystem<T>()`, which calls `SupportsInterface(T::InterfaceName())`.

## Save/Load System

The save pipeline in `GameCore` is layered:

```
IAtlasSaveSystem / IAtlasLoadSystem    ← public interfaces
FAtlasSaveSystem / FAtlasLoadSystem    ← concrete implementations (TSharedFromThis)
FAtlasSaveScheduler                    ← priority queue: Manual > Event > Autosave
FAtlasAutosaveManager                  ← ring-buffer of 5 slots (Autosave_0..4)
FAtlasBinaryWriter / FAtlasBinaryReader ← versioned chunked binary serialization
FAtlasSaveCollector                    ← walks world, collects IAtlasSavable actors
```

**Save data types:**
- `FAtlasSaveGameSnapshot` — flat key/value store (int, float, string, vector, rotator, transform) + `FAtlasSaveGameMetadata`
- `FAtlasWorldSnapshot` → `TArray<FAtlasActorSnapshot>` → `TArray<FAtlasDataChunk>` (named raw binary payloads)

**Actor participation in save/load:**
- Add `UAtlasSavableComponent` to give an actor a stable `FGuid ActorId`
- Implement `IAtlasSavable` (`CaptureState` / `RestoreState`) on the actor or its components
- `UAtlasGASSaveAdapter` — drop-in component that captures/restores `AbilitySystemComponent` attributes and active gameplay effects
- `UAtlasInventorySaveAdapter` — stub adapter for future inventory systems

**Save flow (game thread → async → game thread):**
1. `FAtlasSaveSystem::RequestSave()` enqueues via `FAtlasSaveScheduler`
2. `ProcessNextSaveRequest()` calls `FAtlasSaveCollector` to build `FAtlasWorldSnapshot`
3. `FAtlasBinaryWriter::Serialize()` runs async
4. File write runs async; `HandleSaveCompleted()` fires on game thread

**Load flow (game thread → async → game thread):**
1. `FAtlasLoadSystem::RequestLoad()` reads file async
2. `FAtlasBinaryReader` deserializes async
3. `ApplyWorldSnapshot()` runs on game thread: resolves existing actors by `FGuid`, spawns missing ones, calls `RestoreState()` on each `IAtlasSavable`

## Logging Conventions

All modules must use `GameUtils` macros. Never use `UE_LOG` with raw log categories in production code.

```cpp
ATLAS_LOG_CORE(Warning, "Save slot not found: %s", *SlotName);   // GameCore
ATLAS_LOG_ACTORS(Log, "Player controller initialized");            // GameActors
ATLAS_LOG_UI(Log, "Opened screen: %s", *ScreenName);              // GameUI
ATLAS_LOG_UTILS(Log, "Normalized string: %s", *Result);            // GameUtils
ATLAS_LOG_DEBUG(LogAtlasCore, "High-freq data: %d", Count);       // stripped in Shipping
```

Assertions:
- `ATLAS_CHECK(expr)` — hard crash, programmer error only
- `ATLAS_ENSURE(expr)` — soft, logs and continues
- `ATLAS_ENSURE_MSG(expr, format, ...)` — soft with extra context

## UE C++ Conventions Enforced by This Project

- Module API macro must match module name: `GAMECORE_API`, `GAMEACTORS_API`, `GAMEUI_API`, `GAMEUTILS_API`, `GAME_API`
- Use `TObjectPtr` for `UPROPERTY` owning references; `TWeakObjectPtr` for non-owning; `TSoftObjectPtr` / `TSoftClassPtr` for async-loaded assets
- Pure C++ systems (`IAtlasSystem` subclasses) use `TSharedPtr` / `TSharedFromThis` — they are not `UObject`s
- `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs` in every `.Build.cs`
- `PublicIncludePaths` / `PrivateIncludePaths` must be set explicitly per module; never rely on transitive include pollution
- Prefer `GameInstanceSubsystem` over singletons or static state
- Avoid `Tick` unless strictly necessary

## Adding a New System

1. Define the interface in `GameCore/Public/Systems/<Name>/IAtlas<Name>System.h` extending `IAtlasSystem`. Add `static FName InterfaceName()`.
2. Implement `FAtlas<Name>System` in `GameCore/Private/Systems/<Name>/Atlas<Name>System.cpp` implementing `GetSystemName()`, `SupportsInterface()`, `Initialize()`, `Shutdown()`.
3. Register it in `FAtlasSystemsRegistry::Register()` (`GameCore/Private/Systems/AtlasSystemsRegistry.cpp`) via `Subsystem->RegisterSystem(MakeShared<FAtlas<Name>System>())`.

## Configuration

- Default game instance: `/Script/Game.AtlasGameInstance` (set in `Config/DefaultEngine.ini`)
- Default startup map: `/Engine/Maps/Templates/OpenWorld` (replace when a project map is created)
- Input classes: `EnhancedPlayerInput` + `EnhancedInputComponent` (`Config/DefaultInput.ini`)
- CommonUI accept key handling: `Config/DefaultGame.ini`
- Graphics: DX12, SM6, ray tracing, VSM, Substrate, Lumen — editor/dev only, no shipping config yet