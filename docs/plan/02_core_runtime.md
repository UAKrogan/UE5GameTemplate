# 02 — Core Runtime Architecture

All classes in this section live in `GameCore` unless stated otherwise.

---

## `UAtlasGameInstance`

**Module:** `Game`
**Responsibility:** Project entry point. Resolves and validates the core subsystem. No other logic.
**Blueprint-friendly:** No (C++ only, no UFUNCTION exposure needed).
**Lifecycle:**
- `Init()` → verify `UAtlasGameInstanceSubsystem` exists → log ready
- `Shutdown()` → log shutdown
**Extension points:** Subclass in a game project to override `Init()`/`Shutdown()` if needed.

---

## `UAtlasGameInstanceSubsystem`

**Module:** `GameCore`
**Responsibility:** Central runtime service registry and service locator. Owns all `IAtlasSystem` instances. Controls initialization and shutdown order.
**Blueprint-friendly:** Partially — expose `GetSystem`-equivalent Blueprint utilities via a wrapper function if needed, but the core template method stays C++ only.
**Lifecycle:**
1. `Initialize()` → `RegisterSystems()` → `InitializeSystems()`
2. `Deinitialize()` → `ShutdownSystems()` (reverse order)
**Extension points:**
- `RegisterSystems()` delegates to `FAtlasSystemsRegistry::Register()`.
- Any module may call `RegisterSystem()` if given a pointer to the subsystem — but the registry is the preferred injection point.

**Current state:** Implemented. No changes needed for Phase 1.

---

## `FAtlasSystemsRegistry`

**Module:** `GameCore`
**Responsibility:** The single place where systems are instantiated and registered into the subsystem. Decouples system creation from the subsystem itself.
**Blueprint-friendly:** No.
**Lifecycle:** Called once by `UAtlasGameInstanceSubsystem::RegisterSystems()`.
**Extension points:** Add new `MakeShared<>()` registrations here as systems are implemented. Game Feature plugins should NOT call this directly — they use `UGameFeaturesSubsystem` actions.

**Current state:** Implemented (registers `FAtlasSaveSystem` + `FAtlasLoadSystem`). Extend as systems are added.

---

## `UAtlasGameFlowSubsystem` *(new)*

**Module:** `GameCore`
**Responsibility:** Manages the top-level game state machine: Startup → MainMenu → Gameplay → Paused → ReturnToMenu. Coordinates transitions between game flow states. Notifies other systems (level transition, UI, save) of state changes via delegates.
**Blueprint-friendly:** Yes — expose state query functions and delegates to Blueprints.
**Lifecycle:**
- `Initialize()` → enters `EAtlasGameFlowState::Startup`
- Responds to: startup complete, request main menu, request gameplay, request pause, request return
- Fires delegates: `OnGameFlowStateChanged`
**Extension points:**
- `OnPreEnterState()` / `OnPostEnterState()` virtuals per state
- Delegate broadcast lets `UAtlasUISubsystem` and `UAtlasLevelTransitionSubsystem` react

**Key types:**
```
EAtlasGameFlowState
{
    Startup,
    MainMenu,
    Gameplay,
    Paused,
    Transitioning,
}

FAtlasGameFlowStateChangedDelegate — multicast delegate (OldState, NewState)
```

---

## `UAtlasLevelTransitionSubsystem` *(new)*

**Module:** `GameCore`
**Responsibility:** Handles all level travel: startup map to menu map, menu map to gameplay map, gameplay map to menu map, gameplay map to gameplay map. Manages the transition lifecycle phases.
**Blueprint-friendly:** Yes — expose request methods and state delegates to Blueprints.
**Lifecycle phases per transition:**
1. `RequestTransition(FAtlasLevelTransitionRequest)` — validate, reject if already transitioning
2. Pre-travel phase — notify listeners (save checkpoint, UI pre-travel)
3. Show loading screen
4. `UGameplayStatics::OpenLevel()` (non-seamless for now; seamless travel support added later)
5. Post-load phase — hide loading screen, notify listeners (restore state, spawn HUD)
**Extension points:**
- `FAtlasLevelTransitionRequest` struct with soft map reference, transition type, optional metadata
- `OnPreTravel` / `OnPostLoad` multicast delegates
- Override `PerformTravel()` for seamless/multiplayer travel in game projects

**Key types:**
```cpp
USTRUCT(BlueprintType)
struct FAtlasLevelTransitionRequest
{
    TSoftObjectPtr<UWorld> DestinationMap;
    EAtlasTransitionType TransitionType;  // MainMenu, Gameplay, Restart
    FString TravelOptions;
    bool bShowLoadingScreen;
};

EAtlasTransitionType { ToMainMenu, ToGameplay, RestartLevel }
```

---

## `UAtlasLoadingScreenSubsystem` *(new)*

**Module:** `GameCore` (logic) / `GameUI` (widget)
**Responsibility:** Manages loading screen visibility. Decoupled from level travel so any system can request a loading screen independently.
**Blueprint-friendly:** Yes.
**Lifecycle:**
- `ShowLoadingScreen(FAtlasLoadingScreenConfig)` — pushes loading screen widget on the UI loading layer
- `HideLoadingScreen()` — pops loading screen widget
- `IsLoadingScreenVisible()` — query
**Extension points:**
- `FAtlasLoadingScreenConfig` struct: optional progress info, optional background, optional tips
- Widget class resolved via `UAtlasDeveloperSettings` (soft class reference)
- Driven by `UAtlasLevelTransitionSubsystem` but also callable independently (e.g., long async operations)

---

## `UAtlasAssetManager` *(new)*

**Module:** `GameCore`
**Responsibility:** Custom asset manager for the project. Defines primary asset types, manages async loading, provides helpers for loading pawn data, ability sets, screen definitions, input mapping contexts.
**Blueprint-friendly:** No (C++ only).
**Lifecycle:** Constructed by UE before the game instance; fully owned by the engine.
**Extension points:**
- Override `StartInitialLoading()` to pre-load native gameplay tags or critical data assets
- Static `GetChecked()` helper returning typed reference (Lyra pattern)
- `PrimaryAssetType` constants for: `AtlasPawnData`, `AtlasAbilitySet`, `AtlasScreenDefinition`, `AtlasInputConfig`
**Registration:** Set `AssetManagerClassName=/Script/GameCore.AtlasAssetManager` in `DefaultEngine.ini`.

---

## `UAtlasGameplayTagManager` *(new)*

**Module:** `GameCore`
**Responsibility:** Central place to declare all native gameplay tags used by the framework. Uses `NativeGameplayTags` (UE5 mechanism) for compile-time safe tag references.
**Blueprint-friendly:** Partially — tag constants exposed as `FGameplayTag` statics.
**Lifecycle:** Tags are registered during module startup via `UGameplayTagsManager::Get().AddNativeGameplayTag()`.
**Extension points:**
- Each module may declare its own tag namespace in its own file (e.g., `AtlasTags_Input.h`, `AtlasTags_GAS.h`)
- Game Feature plugins declare their own tags following the same convention

**Tag namespace convention:**
```
Atlas.Input.*           — input-related tags
Atlas.Ability.*         — ability-related tags
Atlas.UI.*              — UI screen/layer tags
Atlas.GameFlow.*        — game state flow tags
Atlas.Save.*            — save/load event tags
Atlas.Feature.*         — feature activation tags
```

---

## `UAtlasDeveloperSettings` *(new)*

**Module:** `GameCore`
**Responsibility:** `UDeveloperSettings` subclass. Editor-configurable project-wide settings for the Atlas framework. Appears in **Project Settings → Atlas Framework**.
**Blueprint-friendly:** Yes — all properties Blueprint-readable.
**Key properties:**

| Property | Type | Purpose |
|---|---|---|
| `StartupMap` | `TSoftObjectPtr<UWorld>` | Map to load at boot |
| `MainMenuMap` | `TSoftObjectPtr<UWorld>` | Main menu map |
| `DefaultGameplayMap` | `TSoftObjectPtr<UWorld>` | Fallback gameplay map |
| `DefaultPawnData` | `TSoftObjectPtr<UAtlasPawnData>` | Fallback pawn data |
| `LoadingScreenWidgetClass` | `TSoftClassPtr<UAtlasLoadingScreenWidget>` | Loading screen widget |
| `DefaultSaveSlotName` | `FString` | Default save slot |
| `AutosaveIntervalSeconds` | `float` | Autosave frequency |
| `bEnableAutosave` | `bool` | Enable timer autosave |
| `bEnableGameFeatures` | `bool` | Enable feature plugin activation |

**Extension points:** Game projects subclass and add game-specific settings.

---

## Feature Activation Support *(new)*

**Module:** `GameCore`
**Class:** `UAtlasFeaturePolicy` (implements `IGameFeatureStateChangeObserver` or uses `UGameFeaturesSubsystem` delegation)
**Responsibility:** Observes Game Feature plugin state changes. Coordinates feature activation with runtime systems (registers additional systems, adds gameplay tags, notifies UI).
**Blueprint-friendly:** No.
**Lifecycle:**
- `Initialize()` → subscribe to `UGameFeaturesSubsystem`
- On feature activated → call registered activation callbacks
- On feature deactivated → call deactivation callbacks
**Extension points:** Feature plugins register themselves via `UGameFeaturesSubsystem` actions, not through this class directly.
