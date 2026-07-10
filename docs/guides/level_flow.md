# Level Flow

Game flow states, level transitions, and loading screens (`GameCore`).

## Flow overview

```
Boot (UAtlasGameInstance::OnStart)
    ↓ StartGameFlow()
Startup ──travel──► MainMenu ──RequestGameplay()──► Gameplay ⇄ Paused
                        ▲                                │
                        └────── RequestReturnToMainMenu ─┘
```

Three subsystems divide the work:

| Subsystem | Owns |
|---|---|
| `UAtlasGameFlowSubsystem` | *What* state the game is in (`Startup`, `MainMenu`, `Gameplay`, `Paused`, `Transitioning`) |
| `UAtlasLevelTransitionSubsystem` | *How* maps change (lifecycle, delegates, re-entrancy guard) |
| `UAtlasLoadingScreenSubsystem` | Loading screen visibility across travel |

## Game flow

```cpp
UAtlasGameFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<UAtlasGameFlowSubsystem>();
Flow->RequestGameplay(nullptr);          // uses DefaultGameplayMap from settings
Flow->RequestPause();  Flow->RequestResume();
Flow->RequestReturnToMainMenu();
Flow->OnGameFlowStateChanged;            // (OldState, NewState)
```

`StartGameFlow()` is called once by the game instance. It travels to `MainMenuMap` (Atlas Framework settings) when set — skipped in PIE so it never hijacks the map being edited; otherwise it enters `MainMenu` in place.

## Requesting a transition

```cpp
FAtlasLevelTransitionRequest Request;
Request.DestinationMap = MyMapSoftPtr;
Request.TransitionType = EAtlasTransitionType::ToGameplay;
Request.bShowLoadingScreen = true;
Request.bSaveCheckpointBeforeTravel = true;      // event-priority checkpoint save
Request.LoadSlotOnPostLoad = TEXT("MySlot");     // optional: restore a save after load

GetGameInstance()->GetSubsystem<UAtlasLevelTransitionSubsystem>()->RequestTransition(Request);
```

Lifecycle per transition: `Idle → PreTravel → Traveling → PostLoad → Idle`.

1. `RequestTransition` validates (rejects re-entrant requests and null maps).
2. `OnPreTravel` broadcasts — close UI, clean up, fade audio.
3. Checkpoint save fires if requested (world state is collected synchronously before travel; the file write finishes async).
4. Loading screen shows; `PerformTravel()` runs `OpenLevel`.
5. Level loads → `NotifyLevelLoaded()` (fired by `PostLoadMapWithWorld` automatically, or explicitly by an `AAtlasPostLoadTrigger` placed in the map).
6. `OnPostLoad` broadcasts — push HUD/menu, restore state. `LoadSlotOnPostLoad` is loaded here if set.
7. Loading screen hides; phase returns to `Idle`. Failures broadcast `OnTransitionFailed`.

## Observers

Subscribe to the dynamic delegates from any system:

```cpp
Transitions->OnPreTravel.AddDynamic(this, &UMyType::HandlePreTravel);   // (Request)
Transitions->OnPostLoad.AddDynamic(this, &UMyType::HandlePostLoad);     // (Request)
```

Typical pre-travel work: request saves, pop screens. Typical post-load work: push the right screen for `Request.TransitionType`, re-apply game state.

## AAtlasPostLoadTrigger

Place one in each project map. On `BeginPlay` it calls `NotifyLevelLoaded()`, decoupling the "level ready" signal from the game mode (disable `bNotifyOnBeginPlay` for maps that signal manually after streaming). The engine `PostLoadMapWithWorld` callback acts as a fallback, and `NotifyLevelLoaded` is idempotent outside the Traveling phase.

## Loading screen

`UAtlasLoadingScreenSubsystem` adds the widget directly to the viewport at Z-order 10000 (above the UI root) and re-adds it after map load, so it stays visible across travel until `HideLoadingScreen()`.

- Widget class: **Atlas Framework settings → UI → LoadingScreenWidgetClass** (a `UAtlasLoadingScreenWidget` subclass from GameUI, stored as a soft class path because GameCore cannot depend on GameUI).
- No class configured: visibility state and `OnLoadingScreenShown/Hidden` delegates still fire so a game-side listener can present its own UI.
- Callable independently of travel for any long async operation.

## Seamless travel (future hook)

Subclass `UAtlasLevelTransitionSubsystem` in your game project and override `PerformTravel()` to use `ServerTravel`/seamless travel. The pre/post lifecycle contract is unchanged.
