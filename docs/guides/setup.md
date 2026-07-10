# New Project Setup

How to take this template and turn it into the base of a real game project.

## 1. Clone and rename (optional)

The project name is `Game` and all framework types carry the `Atlas` prefix. Both are intentionally generic — most projects can keep them and simply build gameplay in feature plugins and game-project subclasses. If you must rename, update `Game.uproject`, the `Source/Game*` folders, `GameTarget.cs`/`GameEditorTarget.cs`, and the `ActiveGameNameRedirects` in `Config/DefaultEngine.ini`.

## 2. Verify config plumbing (already done by the template)

`Config/DefaultEngine.ini`:
```ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/Game.AtlasGameInstance

[/Script/Engine.Engine]
AssetManagerClassName=/Script/GameCore.AtlasAssetManager
```

`Game.uproject` plugins: `GameplayAbilities`, `GameFeatures`, `ModularGameplay`, `CommonUI` — all enabled.

## 3. Create your first maps

1. Create `Content/Maps/MainMenu.umap` and a gameplay map (e.g. `Content/Maps/Playground.umap`).
2. Place an `AAtlasPostLoadTrigger` in each map (signals "level ready" to the transition subsystem).
3. In **Project Settings → Game → Atlas Framework**, set `MainMenuMap` and `DefaultGameplayMap`.
4. Set `GameDefaultMap` in **Project Settings → Maps & Modes** (or `DefaultEngine.ini`) to your main menu map.

On boot, `UAtlasGameInstance::OnStart` → `UAtlasGameFlowSubsystem::StartGameFlow()` travels to `MainMenuMap` automatically (skipped in PIE).

## 4. Set default framework classes

In **Project Settings → Maps & Modes** (or per-map World Settings), set:

- Game Mode: your subclass of `AAtlasGameMode`
- Game State: `AAtlasGameState` (or subclass)
- Player Controller: `AAtlasPlayerController` (or subclass)
- Player State: `AAtlasPlayerState` (or subclass) — this owns the player's ability system
- Default Pawn: driven by pawn data — set `DefaultPawnData` on your game mode instead of hardcoding a pawn class

## 5. Author the pawn data chain

1. `DA_<Game>_AbilitySet` (`UAtlasAbilitySet`) — abilities/effects/attribute sets to grant.
2. `IMC_<Game>_Default` + input actions, then `DA_<Game>_InputConfig` (`UAtlasInputConfigData`) binding actions to `Atlas.Input.Ability.*` tags.
3. `DA_<Game>_PawnData` (`UAtlasPawnData`) — pawn class + ability sets + input configs.
4. Assign the pawn data to your game mode's `DefaultPawnData` and to the pawn's `PawnExtComp`.

See [gas.md](gas.md) for the full initialization flow.

## 6. Set up the UI

1. Create `WBP_AtlasRoot` (subclass `UAtlasRootWidget`) if you want authored layers; otherwise the C++ root builds its layers procedurally.
2. Create screen widgets (subclass `UAtlasMenuWidget` / `UAtlasHUDWidget` / ...).
3. Create one `UAtlasScreenDefinition` per screen and a single `DA_AtlasScreenRegistry` (`UAtlasScreenRegistry`).
4. In **Project Settings → Game → Atlas UI**, set `ScreenRegistry` (and optionally `RootWidgetClass`, `LoadingScreenWidgetClass`).

See [common_ui.md](common_ui.md).

## 7. Configure save/load

**Project Settings → Game → Atlas Framework → Save**: default slot name, autosave toggle/interval, `bSaveCheckpointOnTravel`. Actors participate by adding `UAtlasSavableComponent` and implementing `IAtlasSavable` — see [actors.md](actors.md).

## 8. Build gameplay in feature plugins

Keep the source modules game-agnostic. Put actual gameplay (characters, abilities, screens) in Game Feature plugins under `Plugins/GameFeatures/` — see [game_features.md](game_features.md) and the two `[TEMPLATE EXAMPLE]` sample plugins.
