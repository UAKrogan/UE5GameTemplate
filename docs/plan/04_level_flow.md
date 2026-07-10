# 04 — Level Flow and Loading Architecture

All runtime classes live in `GameCore` unless stated otherwise.

---

## Overview

The level flow system manages the complete lifecycle of map transitions:

```
Startup Boot
    ↓
[Startup map or direct to MainMenu]
    ↓
Main Menu Map
    ↓ (player requests play)
Loading Screen shown
    ↓
Pre-Travel Phase (save checkpoint, notify systems)
    ↓
UE Level Travel (OpenLevel / SeamlessTravel)
    ↓
Post-Load Phase (restore state, spawn HUD, notify systems)
    ↓
Gameplay Map active
    ↓ (player quits / game over)
Pre-Travel Phase (save checkpoint)
    ↓
Travel to Main Menu Map
    ↓
Post-Load Phase (show main menu)
```

---

## Key Types

### `EAtlasTransitionType`

```cpp
UENUM(BlueprintType)
enum class EAtlasTransitionType : uint8
{
    ToMainMenu,       // From gameplay to main menu
    ToGameplay,       // From main menu to gameplay
    RestartLevel,     // Restart the current gameplay level
    ToCustomMap,      // Arbitrary soft-ref map (used by feature plugins)
};
```

### `FAtlasLevelTransitionRequest`

```cpp
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasLevelTransitionRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TSoftObjectPtr<UWorld> DestinationMap;

    UPROPERTY(BlueprintReadWrite)
    EAtlasTransitionType TransitionType = EAtlasTransitionType::ToGameplay;

    UPROPERTY(BlueprintReadWrite)
    FString TravelOptions;

    UPROPERTY(BlueprintReadWrite)
    bool bShowLoadingScreen = true;

    UPROPERTY(BlueprintReadWrite)
    bool bSaveCheckpointBeforeTravel = false;

    UPROPERTY(BlueprintReadWrite)
    FAtlasLoadingScreenConfig LoadingScreenConfig;
};
```

### `EAtlasLevelTransitionPhase`

```cpp
UENUM(BlueprintType)
enum class EAtlasLevelTransitionPhase : uint8
{
    Idle,
    PreTravel,
    Traveling,
    PostLoad,
};
```

---

## `UAtlasLevelTransitionSubsystem`

**Module:** `GameCore`
**Extends:** `UGameInstanceSubsystem`
**Responsibility:** The single authority for level travel. Enforces transition lifecycle, prevents re-entrant transitions, notifies all registered observers.

**Key API:**
```cpp
// Request a transition. Returns false if a transition is already in progress.
bool RequestTransition(const FAtlasLevelTransitionRequest& Request);

// Query
bool IsTransitioning() const;
EAtlasLevelTransitionPhase GetCurrentPhase() const;
const FAtlasLevelTransitionRequest& GetPendingRequest() const;

// Delegates — other systems subscribe to these
FAtlasPreTravelDelegate OnPreTravel;        // fired before OpenLevel
FAtlasPostLoadDelegate OnPostLoad;          // fired after level has loaded
FAtlasTransitionFailedDelegate OnTransitionFailed;

// Called from game mode or from a post-load trigger actor in the new map
void NotifyLevelLoaded();
```

**Internal flow:**
1. `RequestTransition()` — validate, store request, enter `PreTravel` phase
2. Broadcast `OnPreTravel` — subscribers respond (save checkpoint, close UI, cleanup actors)
3. If `bShowLoadingScreen` → call `UAtlasLoadingScreenSubsystem::ShowLoadingScreen()`
4. After pre-travel acks (or timeout) → call `PerformTravel()`
5. `PerformTravel()` → `UGameplayStatics::OpenLevel()` (non-seamless default)
6. After level loads → `NotifyLevelLoaded()` called by a `AAtlasPostLoadActor` placed in each level (or from game mode `BeginPlay`)
7. Broadcast `OnPostLoad` — subscribers respond (restore state, spawn HUD, configure game mode)
8. Hide loading screen

**Seamless travel hook:**
Override `PerformTravel()` in a game project subclass to call `UWorld::SeamlessTravel()` instead. The pre/post lifecycle is identical.

**Multiplayer travel:**
The subsystem is game-instance-level so it runs on both server and client. Server calls `PerformTravel()`. Clients receive travel notification from UE's travel infrastructure. `OnPostLoad` fires independently per machine.

---

## `UAtlasLoadingScreenSubsystem`

**Module:** `GameCore`
**Extends:** `UGameInstanceSubsystem`
**Responsibility:** Manages loading screen visibility. Decoupled from travel so it can be triggered independently.

```cpp
void ShowLoadingScreen(const FAtlasLoadingScreenConfig& Config);
void HideLoadingScreen();
bool IsVisible() const;

// Transition duration between states
float FadeInDuration = 0.25f;
float FadeOutDuration = 0.25f;
```

**Widget management:** The subsystem holds a `TObjectPtr<UAtlasLoadingScreenWidget>` that is added directly to the viewport (bypassing the UI subsystem's stack) so it remains visible during level travel. The widget class is resolved from `UAtlasDeveloperSettings::LoadingScreenWidgetClass`.

**Important:** Loading screen must be added at the `SGameLayerManager` level (above all other widgets) using `UGameViewportClient::AddViewportWidgetContent()` at the highest ZOrder to guarantee visibility during travel.

---

## `AAtlasPostLoadTrigger` *(new, placed in levels)*

**Module:** `GameActors`
**Extends:** `AActor`
**Responsibility:** Placed in each project map. On `BeginPlay`, calls `UAtlasLevelTransitionSubsystem::NotifyLevelLoaded()`. This decouples the "level is ready" signal from the game mode, allowing different game modes per map.
**Blueprint-friendly:** Yes.
**Configuration:** `bNotifyOnBeginPlay = true` (default). Can be disabled for levels that manage the signal manually.

---

## Pre-Travel Phase Detail

Pre-travel phase collects acknowledgements from registered observers before travel actually happens. This allows systems to complete async work (e.g., save in progress, audio fade-out) before the level is torn down.

```cpp
class GAMECORE_API IAtlasPreTravelObserver
{
public:
    virtual FName GetObserverName() const = 0;
    virtual void OnPreTravel(const FAtlasLevelTransitionRequest& Request, FAtlasPreTravelAck Ack) = 0;
};
```

`FAtlasPreTravelAck` is a callback the observer calls when it is done. The subsystem waits for all registered observers to ack, with a configurable timeout (default 5 seconds) to prevent hangs.

Registered observers (examples):
- `FAtlasSaveSystem` — complete any in-progress save
- `UAtlasUISubsystem` — begin screen fade-out
- `UAtlasGameFlowSubsystem` — update flow state

---

## Post-Load Phase Detail

Post-load observers are notified after the new level is ready and the game mode has had `BeginPlay`.

```cpp
class GAMECORE_API IAtlasPostLoadObserver
{
public:
    virtual FName GetObserverName() const = 0;
    virtual void OnPostLoad(const FAtlasLevelTransitionRequest& CompletedRequest) = 0;
};
```

Registered observers (examples):
- `UAtlasUISubsystem` — push appropriate screen (HUD for gameplay, MainMenu for menu)
- `FAtlasLoadSystem` — if transition had a pending load, begin restoring save state
- `UAtlasGameFlowSubsystem` — enter correct game flow state

---

## Map Configuration

Maps are never hardcoded. They are configured in `UAtlasDeveloperSettings`:

```ini
[/Script/GameCore.AtlasDeveloperSettings]
StartupMap=/Game/Maps/Startup.Startup
MainMenuMap=/Game/Maps/MainMenu.MainMenu
DefaultGameplayMap=/Game/Maps/DefaultLevel.DefaultLevel
```

Game Feature plugins may override the default gameplay map by injecting a different `FAtlasLevelTransitionRequest` before travel begins.

---

## Future Seamless Travel

To support seamless travel (required for multiplayer level streaming or lobby-to-game transitions):

1. Override `PerformTravel()` in a game project subclass of `UAtlasLevelTransitionSubsystem`
2. Call `AGameModeBase::bUseSeamlessTravel = true` in the game mode
3. Call `UWorld::ServerTravel()` on the server instead of `OpenLevel`
4. Implement `AAtlasGameMode::GetSeamlessTravelActorList()` to persist persistent actors

The pre/post observer contract is identical — only `PerformTravel()` changes.

---

## Future Multiplayer Travel

For multiplayer-hosted sessions:

1. Server requests transition via `UAtlasLevelTransitionSubsystem::RequestTransition()`
2. Pre-travel phase on server only (save, cleanup)
3. Server calls `ServerTravel` with `bAbsolute = false` for session persistence
4. Clients receive `TravelURL` notification from NetDriver
5. Client-side `UAtlasLevelTransitionSubsystem` fires its own `OnPostLoad` after client level loads

No shared state is needed between server and client transition subsystems beyond what UE's travel infrastructure provides.

---

## Startup Flow Sequence

```
UAtlasGameInstance::Init()
  → UAtlasGameFlowSubsystem enters EAtlasGameFlowState::Startup
  → UAtlasLoadingScreenSubsystem::ShowLoadingScreen()
  → UAtlasAssetManager::StartInitialLoading() finishes
  → (Optional) Play startup cinematic / splash
  → UAtlasGameFlowSubsystem requests transition to MainMenu
  → UAtlasLevelTransitionSubsystem::RequestTransition({ MainMenuMap, ToMainMenu })
  → Travel to MainMenuMap
  → OnPostLoad: UAtlasUISubsystem.PushScreen("MainMenu")
  → UAtlasLoadingScreenSubsystem::HideLoadingScreen()
  → UAtlasGameFlowSubsystem enters EAtlasGameFlowState::MainMenu
```
