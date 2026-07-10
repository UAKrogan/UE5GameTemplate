# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Identity

**UE5GameTemplate** is an Unreal Engine 5.7 C++ multi-module framework (`Atlas` prefix) intended as a reusable base for different games. It is not a game itself. Never add game-specific mechanics (combat, inventory, quests, economy, etc.) unless explicitly asked for an isolated example.

Engine: `5.7` | Project descriptor: `Game.uproject` | Targets: `GameTarget`, `GameEditorTarget`

Enabled plugins: `GameplayAbilities`, `GameFeatures`, `ModularGameplay`, `CommonUI` (+ `ModelingToolsEditorMode`, editor-only).

## Build Commands

All builds go through UnrealBuildTool. There is no standalone CLI build script; use the IDE or UBT directly.

```
"<UE5_ROOT>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" GameEditor Win64 Development "D:\Projects\UnrealEngine\UE5GameTemplate\Game.uproject"
```

(Use target `Game` for the non-editor build. Local engine root: `D:\UnrealEngine\UE_5.7`.)

There are no automated tests yet. When added they should use Unreal's Automation framework (`FAutomationTestBase`).

## Module Architecture

The allowed dependency graph is strictly one-way — modules may only depend on those to their right:

```
Game → GameActors, GameCore, GameUI, GameUtils
GameActors → GameUtils  (+ GAS modules, EnhancedInput, AIModule, ModularGameplay, GameFeatures)
GameCore → GameUtils  (+ GAS modules, DeveloperSettings; private: UMG/Slate for loading widget, Json)
GameUI → GameUtils  (+ UMG, CommonUI, CommonInput, InputCore, DeveloperSettings, GameFeatures)
GameUtils → (no local deps)
```

**Never create cross-module dependencies that go against this graph.** GameActors/GameUI/GameCore must never depend on each other. Feature plugins and the `Game` host module are the only places that may depend on multiple local modules simultaneously. When a plan-level design conflicts with this graph (it happens), the graph wins — bridge with soft object paths, delegates, or data assets.

### Module Responsibilities

| Module | Purpose |
|---|---|
| `Game` | Thin host. `UAtlasGameInstance` bootstrap (`Init` + `OnStart` → `StartGameFlow`). `IMPLEMENT_PRIMARY_GAME_MODULE`. |
| `GameCore` | Runtime systems + subsystem orchestration: `UAtlasGameInstanceSubsystem` (service locator), game flow / level transition / loading screen subsystems, `UAtlasAssetManager`, `UAtlasDeveloperSettings`, native tags (`AtlasGameplayTags`), save/load pipeline, `AAtlasPostLoadTrigger`. |
| `GameActors` | GAS-ready base classes (`AAtlasCharacter`, `AAtlasPawn`, controllers, game mode/state, `AAtlasPlayerState` owning the player ASC), extension components (PawnExt/AbilityExt/InputExt/MovementExt), `UAtlasAbilitySystemComponent`, `UAtlasBaseAttributeSet`, `UAtlasBaseGameplayAbility`, data assets (`UAtlasPawnData`, `UAtlasAbilitySet`, `UAtlasInputConfigData`), feature actions (AddAbilities, AddInputConfig). |
| `GameUI` | `UAtlasUISubsystem` (layered screen stack), `UAtlasRootWidget` + base widgets, `UAtlasScreenDefinition`/`UAtlasScreenRegistry`, `UAtlasUIDeveloperSettings`, `UAtlasGlyphSubsystem`, feature actions (AddScreens, AddHUDElements). No widget assets — authored per game. |
| `GameUtils` | Log categories, logging macros, assertion helpers. Nothing else. |

## Runtime Lifecycle

`UAtlasGameInstance::Init()` → resolves `UAtlasGameInstanceSubsystem`; `OnStart()` → `UAtlasGameFlowSubsystem::StartGameFlow()` (auto-travel to MainMenuMap; skipped in PIE).

`UAtlasGameInstanceSubsystem` orchestrates pure C++ systems:
1. `Initialize()` → `RegisterSystems()` (delegates to `FAtlasSystemsRegistry`) → `InitializeSystems()`
2. `Deinitialize()` → `ShutdownSystems()` (reverse registration order)

Systems are stored as `TMap<FName, TSharedPtr<IAtlasSystem>>` and looked up by interface via `GetSystem<T>()`, which calls `SupportsInterface(T::InterfaceName())`.

Engine subsystems (`UGameInstanceSubsystem`) are used where UObject lifecycle/Blueprint access matters: `UAtlasGameFlowSubsystem` (state machine), `UAtlasLevelTransitionSubsystem` (PreTravel→Traveling→PostLoad with `OnPreTravel`/`OnPostLoad`/`OnTransitionFailed`), `UAtlasLoadingScreenSubsystem` (viewport-level widget surviving travel), `UAtlasUISubsystem` (GameUI).

## GAS Setup

- Player pawns: ASC lives on `AAtlasPlayerState` (survives respawn); AI pawns: pawn-owned ASC created by `UAtlasAbilityExtensionComponent` from `AAtlasAIController::OnPossess`.
- `UAtlasPawnExtensionComponent` is the init hub: possession hooks (`PossessedBy`/`OnRep_Controller`/`OnRep_PlayerState`) call `HandleControllerChanged()`; grants `UAtlasPawnData` ability sets on authority; fires `OnAbilitySystemInitialized`.
- Input: tag-based (`Atlas.Input.Ability.*`). `UAtlasInputExtensionComponent` (on the player controller, wired from the pawn's `SetupPlayerInputComponent`) binds input actions from `UAtlasInputConfigData` to `AbilityInputTagPressed/Released` on the ASC. Input never references ability classes.
- All base actors are Modular Gameplay receivers (`AddGameFrameworkComponentReceiver` in `PreInitializeComponents`, `NAME_GameActorReady` in `BeginPlay`, remove in `EndPlay`).

## UI System

Screens are pushed by ID: `UAtlasUISubsystem::PushScreen(FName)` resolves a `UAtlasScreenDefinition` through the registry configured in **Atlas UI** settings (`UAtlasUIDeveloperSettings`, in GameUI — not GameCore, because GameUI must not depend on GameCore). Root widget layers: Game < Menu < Modal < Notification < Loading; constructed procedurally when no Blueprint root is configured. Input mode is derived from the top-most active screen.

## Save/Load System

The save pipeline in `GameCore` is layered:

```
IAtlasSaveSystem / IAtlasLoadSystem    ← public interfaces
FAtlasSaveSystem / FAtlasLoadSystem    ← concrete implementations (TSharedFromThis)
FAtlasSaveScheduler                    ← priority queue: Manual > Event > Autosave
FAtlasAutosaveManager                  ← ring-buffer of 5 slots (Autosave_0..4)
FAtlasBinaryWriter / FAtlasBinaryReader ← versioned chunked binary serialization
FAtlasSaveCollector                    ← walks world, collects IAtlasSavable actors
FAtlasSaveMigrationManager             ← sequential vN→vN+1 snapshot migrations
FAtlasSaveSlotManifest                 ← JSON slot manifest (SaveManifest.json)
```

**Actor participation:** add `UAtlasSavableComponent` (stable `FGuid ActorId`), implement `IAtlasSavable` (`CaptureState`/`RestoreState`); `UAtlasGASSaveAdapter` covers ASC attributes/effects.

**Flow:** requests are game-thread; world collection is synchronous; serialization + file I/O run async and complete back on the game thread. `bSaveInProgress`/`bLoadInProgress` are `std::atomic<bool>`. Level transitions can checkpoint before travel (`bSaveCheckpointBeforeTravel`) and restore a slot post-load (`LoadSlotOnPostLoad`).

## Game Feature Plugins

Live under `Plugins/GameFeatures/`; depend on source modules, never the reverse, never on each other. Custom actions: `Atlas: Add Abilities`, `Atlas: Add Input Config` (GameActors), `Atlas: Add Screens`, `Atlas: Add HUD Elements` (GameUI). Two `[TEMPLATE EXAMPLE]` content-only sample plugins exist; their READMEs list the editor assets to author.

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
- Native tags live in `GameCore/Public/Tags/AtlasGameplayTags.h` (`Atlas.Category.Subcategory` convention)
- Full conventions: `docs/guides/conventions.md`

## Adding a New System

1. Define the interface in `GameCore/Public/Systems/<Name>/IAtlas<Name>System.h` extending `IAtlasSystem`. Add `static FName InterfaceName()`.
2. Implement `FAtlas<Name>System` in `GameCore/Private/Systems/<Name>/Atlas<Name>System.cpp` implementing `GetSystemName()`, `SupportsInterface()`, `Initialize()`, `Shutdown()`.
3. Register it in `FAtlasSystemsRegistry::Register()` (`GameCore/Private/Systems/AtlasSystemsRegistry.cpp`) via `Subsystem->RegisterSystem(MakeShared<FAtlas<Name>System>())`.

## Configuration

- Default game instance: `/Script/Game.AtlasGameInstance`; asset manager: `/Script/GameCore.AtlasAssetManager` (both `Config/DefaultEngine.ini`)
- Framework settings: `[/Script/GameCore.AtlasDeveloperSettings]` in `Config/DefaultGame.ini` (maps, save, features) — Project Settings → Game → Atlas Framework
- UI settings: `UAtlasUIDeveloperSettings` — Project Settings → Game → Atlas UI (screen registry, root/loading/notification widget classes, glyphs)
- Default startup map: `/Engine/Maps/Templates/OpenWorld` (replace when project maps are created)
- Input classes: `EnhancedPlayerInput` + `EnhancedInputComponent` (`Config/DefaultInput.ini`)
- Graphics: DX12, SM6, ray tracing, VSM, Substrate, Lumen — editor/dev only, no shipping config yet

## Docs

- `docs/guides/` — user-facing guides (start at `README.md` there); these describe the implemented behavior and are the source of truth
- `docs/plan/` — only open future work remains: `06_movement.md` (vehicles, mounts, camera config — unimplemented) plus a README listing smaller remnants. Implemented plans (00–13) were removed after the phased implementation and live in git history
