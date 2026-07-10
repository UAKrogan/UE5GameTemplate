# 03 — UI Architecture

All classes live in `GameUI` unless stated otherwise.

---

## Philosophy

- Common UI is the base. Never use bare `UUserWidget` for navigable screens.
- All screens are registered in a data asset (`UAtlasScreenRegistry`), not hardcoded in C++.
- The UI subsystem manages a layered stack. Callers push/pop screen IDs. The subsystem resolves widget classes.
- Input mode is owned by the screen stack, not by individual widgets or the player controller.
- Controller navigation (gamepad) is handled by Common UI's focus system; the subsystem configures it.
- Platform glyph switching is data-driven.

---

## `UAtlasUISubsystem` (extend existing)

**Responsibility:** Central UI runtime service. Manages layers and screen stacks. Bridges between game systems and UI widgets. Handles input mode switching.
**Blueprint-friendly:** Yes — primary API is Blueprint-callable.
**Lifecycle:**
- `Initialize()` → create root widget, add to viewport, initialize layers
- `Deinitialize()` → remove root widget, clear layers
**Key API:**
```cpp
void PushScreen(FName ScreenId, UObject* Context = nullptr);
void PopScreen(FName ScreenId);
void PopAllScreens();

void ShowHUD();
void HideHUD();

void ShowModal(FName ModalId, FAtlasModalPayload Payload);
void DismissModal();

void ShowNotification(FAtlasNotificationPayload Payload);

void ShowLoadingScreen(FAtlasLoadingScreenConfig Config);
void HideLoadingScreen();

void SetInputMode(EAtlasInputMode Mode);
EAtlasInputMode GetCurrentInputMode() const;

UFUNCTION(BlueprintCallable)
UAtlasActivatableWidget* GetActiveScreen() const;
```
**Extension points:**
- `OnScreenPushed` / `OnScreenPopped` delegates
- `OnInputModeChanged` delegate
- Override `CreateRootWidget()` to provide a custom root

---

## Layer System

The root widget (`UAtlasRootWidget`) contains named widget layers as `UCommonActivatableWidgetStack` or `UOverlay` slots:

```
UAtlasRootWidget (UCommonUserWidget)
├── GameLayer      (UCommonActivatableWidgetStack) — gameplay HUD
├── MenuLayer      (UCommonActivatableWidgetStack) — menus, pause, settings
├── ModalLayer     (UCommonActivatableWidgetStack) — modal dialogs
├── NotificationLayer (UOverlay)                  — toasts/notifications (non-blocking)
└── LoadingLayer   (UOverlay)                     — loading screen (always on top)
```

Layer Z-order: Loading > Notification > Modal > Menu > Game.

`UAtlasUILayerDefinition` — data asset defining a layer's name, widget stack class, and Z-order.

---

## `UAtlasActivatableWidget` (base for all screens)

**Extends:** `UCommonActivatableWidget`
**Responsibility:** Base class for all Atlas navigable widgets. Provides Atlas-specific lifecycle hooks.
**Key overrides/additions:**
```cpp
virtual void NativeOnActivated() override;
virtual void NativeOnDeactivated() override;

// Called when the screen receives context data
virtual void InitializeWithContext(UObject* Context);

// Returns the input mode this screen requests
virtual EAtlasInputMode GetRequestedInputMode() const;

// Returns whether back navigation should pop this screen
virtual bool GetRequestsDismissOnBack() const;
```

---

## `UAtlasScreenDefinition` (data asset)

**Module:** `GameUI`
**Responsibility:** Defines a single screen: what widget class to spawn, which layer to push it on, what input mode it requests.
**Properties:**
```cpp
FName ScreenId;
TSoftClassPtr<UAtlasActivatableWidget> WidgetClass;
EAtlasUILayer TargetLayer;
EAtlasInputMode RequestedInputMode;
bool bSingleInstance;
```

---

## `UAtlasScreenRegistry` (data asset)

**Module:** `GameUI`
**Responsibility:** Maps screen IDs to `UAtlasScreenDefinition` assets. Referenced by `UAtlasUISubsystem`.
**Properties:**
```cpp
TMap<FName, TSoftObjectPtr<UAtlasScreenDefinition>> Screens;
```
**Extension:** Game Feature plugins contribute additional screen definitions via `GameFeatureAction_AddScreens` (custom action).

---

## HUD Layer — `UAtlasHUDWidget`

**Extends:** `UAtlasActivatableWidget`
**Layer:** `GameLayer`
**Input mode:** `Game`
**Responsibility:** Root HUD widget. Contains slots for ability indicators, health/resource bars, minimap, objective display. All HUD content is added via named named widget slots, not hardcoded children.
**Extension points:**
- `AddHUDElement(FName SlotName, UAtlasActivatableWidget* Element)` — allows Game Feature plugins to inject HUD elements
- `RemoveHUDElement(FName SlotName)`

---

## Menu Layer Screens

### `UAtlasMainMenuWidget`
- Layer: `MenuLayer`
- Input mode: `Menu`
- Responsibility: Main menu root. Buttons navigate to sub-screens (Play, Settings, Quit).

### `UAtlasPauseMenuWidget`
- Layer: `MenuLayer`
- Input mode: `Menu`
- Responsibility: Pause root. Resume, Settings, Return to Main Menu.

### `UAtlasSettingsWidget`
- Layer: `MenuLayer`
- Input mode: `Menu`
- Responsibility: Settings/options. Tabbed layout (Gameplay, Graphics, Audio, Controls).

---

## Modal Layer — `UAtlasModalWidget`

**Extends:** `UAtlasActivatableWidget`
**Layer:** `ModalLayer`
**Input mode:** `Menu`
**Responsibility:** Generic confirmation dialog or info dialog.
**Payload:**
```cpp
USTRUCT(BlueprintType)
struct FAtlasModalPayload
{
    FText Title;
    FText Body;
    FText ConfirmLabel;
    FText CancelLabel;
    bool bShowCancel;
    TFunction<void(bool bConfirmed)> OnResult;
};
```

---

## Notification Layer — `UAtlasNotificationWidget`

**Extends:** `UUserWidget` (notifications are non-blocking, not activatable)
**Layer:** `NotificationLayer`
**Responsibility:** Toast-style transient notification. Auto-dismisses after a configured duration.
**Payload:**
```cpp
USTRUCT(BlueprintType)
struct FAtlasNotificationPayload
{
    FText Message;
    float DisplayDurationSeconds;
    EAtlasNotificationType Type;  // Info, Warning, Success
};
```

---

## Loading Layer — `UAtlasLoadingScreenWidget`

**Extends:** `UUserWidget` (not activatable — must always be addable even without a valid player)
**Layer:** `LoadingLayer`
**Responsibility:** Full-screen loading screen. Can display progress, tips, background.
**Config:**
```cpp
USTRUCT(BlueprintType)
struct FAtlasLoadingScreenConfig
{
    TSoftObjectPtr<UTexture2D> BackgroundImage;
    FText Tip;
    bool bShowProgress;
    float FadeInDuration;
};
```

---

## Input Mode Management

`EAtlasInputMode` — controls what receives input:
```cpp
enum class EAtlasInputMode : uint8
{
    Game,           // game input only, cursor hidden
    Menu,           // UI input only, cursor visible
    GameAndMenu,    // both, cursor visible (e.g. in-game inventory)
    GamepadCursor,  // gamepad controls a cursor over UI
};
```

`UAtlasUISubsystem::SetInputMode()` calls the appropriate combination of `FInputModeGameOnly`, `FInputModeUIOnly`, `FInputModeGameAndUI` on the local player controller. The input mode stack is maintained so screens can restore the previous mode on deactivation.

---

## Controller Navigation and Gamepad Support

Common UI handles gamepad navigation natively via `UCommonActivatableWidget` focus rules. The Atlas layer adds:
- `UAtlasUISubsystem` sets `bIsFocusable = true` on menu screens
- Gamepad back button mapped to `CommonUI.Back` action
- Navigation config per screen stored in `UAtlasScreenDefinition`

---

## Xbox / PlayStation Glyph Strategy

**Approach:** Data-driven platform detection, not hardcoded sprite switching.

1. `UAtlasInputGlyphData` — data asset mapping `FKey` → platform-specific texture/material
2. `UAtlasGlyphSubsystem` (local player subsystem) — detects current input device (keyboard, Xbox, PlayStation) via `FSlateApplication::Get().GetInputKeyboard()` / latest input event tracking
3. `UAtlasGlyphWidget` — a small widget that takes a `FKey` or `FGameplayTag(input action)` and displays the correct glyph, auto-updating when input device changes
4. On device change → `UAtlasGlyphSubsystem` broadcasts `OnInputDeviceChanged(EAtlasInputDevice)` → all `UAtlasGlyphWidget` instances re-bind

`EAtlasInputDevice { KeyboardMouse, Xbox, PlayStation, Generic }`

This allows any screen or HUD element to show the correct "Press [A] to confirm" / "Press [Cross] to confirm" glyph without hardcoding per-platform widgets.

---

## Screen Flow Summary

### Startup Flow
```
Game boots → UAtlasGameFlowSubsystem enters Startup
→ Show loading screen
→ Load startup map (or skip if already on startup map)
→ After assets loaded → transition to MainMenu state
→ Hide loading screen
→ UAtlasUISubsystem.PushScreen("MainMenu")
```

### Main Menu → Gameplay
```
Player presses Play
→ UAtlasGameFlowSubsystem requests gameplay transition
→ UAtlasLevelTransitionSubsystem.RequestTransition(GameplayMap)
→ UISubsystem.ShowLoadingScreen()
→ PopAllScreens() (clears main menu)
→ OpenLevel(GameplayMap)
→ Post-load: HideLoadingScreen(), PushScreen("HUD")
```

### Pause
```
Player presses Pause
→ UAtlasGameFlowSubsystem enters Paused
→ UISubsystem.PushScreen("PauseMenu")
→ InputMode = Menu
Resume: UISubsystem.PopScreen("PauseMenu") → InputMode = Game
```

### Return to Main Menu
```
Player selects Return to Main Menu
→ UISubsystem.ShowLoadingScreen()
→ UISubsystem.PopAllScreens()
→ FAtlasSaveSystem: checkpoint save if configured
→ OpenLevel(MainMenuMap)
→ Post-load: HideLoadingScreen(), PushScreen("MainMenu")
```
