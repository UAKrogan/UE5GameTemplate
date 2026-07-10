# Common UI Layer

How screens, layers, modals, notifications, and glyphs work in `GameUI`.

## Principles

- Common UI is the base: navigable screens extend `UAtlasActivatableWidget` (a `UCommonActivatableWidget`), never bare `UUserWidget`.
- Screens are registered in a data asset (`UAtlasScreenRegistry`), not hardcoded — callers push/pop **screen IDs**.
- Input mode is owned by the screen stack, not by widgets or the player controller.

## Layer system

`UAtlasRootWidget` owns five layers, bottom to top:

```
UAtlasRootWidget
├── GameLayer          (UCommonActivatableWidgetStack)  — gameplay HUD
├── MenuLayer          (UCommonActivatableWidgetStack)  — menus, pause, settings
├── ModalLayer         (UCommonActivatableWidgetStack)  — dialogs
├── NotificationLayer  (UOverlay)                       — toasts (non-blocking)
└── LoadingLayer       (UOverlay)                       — loading screen (top)
```

A Blueprint subclass can bind these by name (`BindWidgetOptional`); with the raw C++ class the hierarchy is constructed procedurally, so the UI works with zero content assets. The root widget is created lazily by `UAtlasUISubsystem` and re-added to the viewport after level travel.

## Registering a screen

1. Create a widget Blueprint subclassing `UAtlasMenuWidget` (or another base).
2. Create a `UAtlasScreenDefinition`: `ScreenId`, `WidgetClass`, `TargetLayer`, `RequestedInputMode`, `bSingleInstance`.
3. Add it to the project's `UAtlasScreenRegistry` under its ID.
4. Point **Project Settings → Game → Atlas UI → ScreenRegistry** at the registry.

Game Feature plugins register screens at runtime via the `Atlas: Add Screens` action instead of touching the authored registry.

## Push / pop

```cpp
UAtlasUISubsystem* UI = GetGameInstance()->GetSubsystem<UAtlasUISubsystem>();
UI->PushScreen("PauseMenu");            // resolves definition, pushes on its layer
UI->PopScreen("PauseMenu");             // pops the top-most instance
UI->PopAllScreens();
UI->IsScreenActive("PauseMenu");
UI->GetActiveScreen();                  // top-most across layers
```

`OnScreenPushed` / `OnScreenPopped` broadcast the screen ID. Back navigation (Common UI back handling) deactivates the widget; the subsystem's bookkeeping and input mode update automatically.

## Input modes

`EAtlasInputMode`: `Game`, `Menu`, `GameAndMenu`, `GamepadCursor`. After every push/pop the subsystem applies the input mode requested by the top-most screen (highest layer wins), falling back to `Game` when no screens remain. `OnInputModeChanged` broadcasts changes.

## HUD

`ShowHUD()` / `HideHUD()` push/pop the screen ID configured as `HUDScreenId` (default `HUD`) in Atlas UI settings. `UAtlasHUDWidget` exposes named element slots:

```cpp
HUD->AddHUDElement("HealthBar", Widget);   // BP_OnHUDElementAdded decides placement
HUD->RemoveHUDElement("HealthBar");
```

Feature plugins inject elements with the `Atlas: Add HUD Elements` action.

## Modals

```cpp
FAtlasModalPayload Payload;
Payload.Title = INVTEXT("Quit?");
Payload.bShowCancel = true;

FAtlasModalResultDelegate OnResult;
OnResult.BindDynamic(this, &UMyClass::HandleQuitResult);   // (bool bConfirmed)

UI->ShowModal("ConfirmDialog", Payload, OnResult);
```

The modal screen must subclass `UAtlasModalWidget`; its Blueprint binds buttons to `Confirm()` / `Cancel()`. `DismissModal()` cancels the active modal.

## Notifications

```cpp
FAtlasNotificationPayload Payload;
Payload.Message = INVTEXT("Game saved");
Payload.DisplayDurationSeconds = 3.0f;
UI->ShowNotification(Payload);
```

Notification widgets (subclass `UAtlasNotificationWidget`, class configured in Atlas UI settings) are added to the Notification overlay and auto-removed after the duration.

## Loading screen

`UAtlasUISubsystem::ShowLoadingScreen/HideLoadingScreen` drive the root widget's Loading layer for in-place waits. For level travel, use `UAtlasLoadingScreenSubsystem` (GameCore) instead — it adds the widget at the viewport level so it survives travel; see [level_flow.md](level_flow.md).

## Controller glyphs

`UAtlasGlyphSubsystem` (local player subsystem) wraps Common Input device detection and maps it to `EAtlasInputDevice` (KeyboardMouse / Xbox / PlayStation / Generic). Configure a `UAtlasInputGlyphData` asset (key → texture per device) in Atlas UI settings, then:

```cpp
UTexture2D* Glyph = GlyphSubsystem->GetGlyphForKey(EKeys::Gamepad_FaceButton_Bottom);
GlyphSubsystem->OnInputDeviceChanged.AddDynamic(this, &UMyWidget::HandleDeviceChanged);
```
