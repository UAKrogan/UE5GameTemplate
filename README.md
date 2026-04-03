# Game

`Game` is an Unreal Engine 5.7 C++ project organized around a small Atlas runtime architecture. The current codebase provides module boundaries, gameplay framework base classes, game-instance subsystems, and a simple runtime systems registry for save/load services. Content is still minimal and the default Open World template map remains the project entry map.

## Current State

- Engine association: `5.7`
- Project descriptor: `Game.uproject`
- Runtime modules declared in `Game.uproject`:
  - `Game`
  - `GameActors`
  - `GameCore`
  - `GameUI`
  - `GameUtils`
- Enabled project plugin: `ModelingToolsEditorMode` for editor targets
- Local developer plugin present: `Plugins/Developer/RiderLink`
- Default game instance: `UAtlasGameInstance`
- Default startup map: `/Engine/Maps/Templates/OpenWorld`

The project is no longer an empty modular scaffold. It now includes:

- A runtime bootstrap path through `UAtlasGameInstance`
- A `UGameInstanceSubsystem` that owns and initializes runtime systems
- Registered save/load systems with a shared `IAtlasSystem` interface
- A separate UI subsystem for screen management hooks
- Base gameplay framework classes for character, pawn, controllers, game mode, game state, and player state
- Shared logging categories and logging/assertion macros

What is still missing is real gameplay behavior, authored UI widgets, custom maps, and production content.

## Architecture Overview

### Runtime bootstrap

`UAtlasGameInstance` is configured as the project game instance in `Config/DefaultEngine.ini`.

At startup it:

1. Calls `Super::Init()`
2. Logs initialization
3. Resolves `UAtlasGameInstanceSubsystem`
4. Verifies the subsystem exists

### Core orchestration

`UAtlasGameInstanceSubsystem` in `GameCore` is the central runtime orchestrator.

Its lifecycle is:

1. `Initialize()`
2. `RegisterSystems()`
3. `InitializeSystems()`
4. `Deinitialize()`
5. `ShutdownSystems()`

It stores systems in a `TMap<FName, TSharedPtr<IAtlasSystem>>` and exposes templated lookup via `GetSystem<T>()`.

### Systems registry

`FAtlasSystemsRegistry` registers systems into the game-instance subsystem. The current registry adds:

- `FAtlasSaveSystem`
- `FAtlasLoadSystem`

Both systems implement `IAtlasSystem`, are initialized once per session, and currently only log their lifecycle and entry-point calls (`SaveGame()` / `LoadGame()`).

### UI layer

`UAtlasUISubsystem` is a separate `UGameInstanceSubsystem` intended for UI flow. It currently exposes:

- `OpenScreen(FName ScreenId)`
- `CloseScreen()`

These methods currently log actions only; there are no widgets or screen assets in the repository yet.

## Module Layout

```text
Source/
|-- Game/                   # Primary module and game instance bootstrap
|-- GameActors/             # Gameplay framework base classes
|-- GameCore/               # Runtime subsystem + systems registry/interfaces
|-- GameUI/                 # UI subsystem scaffold
|-- GameUtils/              # Shared logging and utility layer
|-- Game.Target.cs
`-- GameEditor.Target.cs
```

### `Game`

Purpose:
- Primary game module
- Defines `UAtlasGameInstance`
- Depends on all local runtime modules

Key implementation:
- `IMPLEMENT_PRIMARY_GAME_MODULE`
- Startup/shutdown logging
- `AtlasGameInstance` runtime bootstrap

### `GameActors`

Purpose:
- Home for gameplay framework base classes

Current classes:
- `AAtlasCharacter`
- `AAtlasPawn`
- `AAtlasPlayerController`
- `AAtlasAIController`
- `AAtlasGameMode`
- `AAtlasGameState`
- `AAtlasPlayerState`

Current behavior:
- Classes mostly provide overridable initialization hooks such as `InitializeCharacter()`, `InitializeController()`, and `InitializeGameMode()`
- No gameplay logic, input bindings, movement rules, AI behavior, or possession flow is implemented yet

Dependencies:
- `InputCore`
- `EnhancedInput`
- `AIModule`
- `GameUtils`

### `GameCore`

Purpose:
- Owns runtime service abstractions and the main game-instance subsystem

Current contents:
- `IAtlasSystem`
- `IAtlasSaveSystem`
- `IAtlasLoadSystem`
- `FAtlasSaveSystem`
- `FAtlasLoadSystem`
- `FAtlasSystemsRegistry`
- `UAtlasGameInstanceSubsystem`

Design intent:
- Keep systems actor-independent
- Centralize system ownership in the subsystem
- Avoid circular dependencies by registering systems through the registry

### `GameUI`

Purpose:
- Central place for UI runtime flow

Current contents:
- `UAtlasUISubsystem`

Dependencies:
- `UMG`
- `CommonUI`
- `Slate`
- `SlateCore`
- `GameUtils`

Current status:
- No widget classes or UI assets checked in yet

### `GameUtils`

Purpose:
- Shared low-level functionality across modules

Current contents:
- Log categories
- Logging macros
- Assertion helper macros
- Debug-only logging helper

Log categories:
- `LogAtlas`
- `LogAtlasActors`
- `LogAtlasCore`
- `LogAtlasUI`
- `LogAtlasUtils`

## Configuration Snapshot

### `DefaultEngine.ini`

Current engine configuration includes:

- `GameDefaultMap=/Engine/Maps/Templates/OpenWorld`
- `GameInstanceClass=/Script/Game.AtlasGameInstance`
- Desktop hardware targeting
- Maximum graphics profile
- `DefaultGraphicsRHI_DX12`
- Shader Model 6 for D3D12
- Static lighting disabled
- Mesh distance fields enabled
- Dynamic GI enabled
- Reflection method enabled
- Ray tracing enabled
- Substrate enabled
- Virtual shadow maps enabled

The file also contains Android File Server settings and Unreal template redirect entries from `TP_Blank` to `Game`.

### `DefaultInput.ini`

Current input configuration sets:

- `EnhancedPlayerInput` as the default player input class
- `EnhancedInputComponent` as the default input component class
- Standard axis configs for mouse, gamepad, and XR devices

There are currently no custom input actions, input mapping contexts, or project-specific bindings implemented in code or content.

### `DefaultGame.ini`

Current game configuration contains:

- `CommonUI` accept key handling
- Unreal-generated `ProjectID`

### `DefaultEditor.ini`

Currently empty.

## Content Layout

`Content/` currently contains only:

- `Collections/`
- `Developers/`
- `Developers/uakro/Collections/`

There are no shipped game assets, no project map, no Blueprint gameplay classes, no widget Blueprints, and no data assets committed outside developer folders.

## Build And Open

### Open in Unreal Editor

Open `Game.uproject` with Unreal Engine 5.7.

### Generate IDE files

Project files are not committed at the moment. Generate them from the `.uproject` when needed.

Typical options:

- Unreal Editor or UnrealVersionSelector: generate Visual Studio/Rider project files from `Game.uproject`
- Rider: open the folder or open `Game.uproject`
- Visual Studio: generate project files first, then open the generated solution

### Targets

- `GameTarget` for runtime builds
- `GameEditorTarget` for editor builds

Both targets use:

- `BuildSettingsVersion.V6`
- `EngineIncludeOrderVersion.Unreal5_7`

## Repository Notes

The repository already includes a `.gitignore` covering common Unreal generated output such as:

- `.idea/`
- `.vs/`
- `Binaries/`
- `Build/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- generated solution/project artifacts

The working tree may still contain local generated folders depending on the machine state. In this repository snapshot, folders such as `Binaries/`, `DerivedDataCache/`, `Intermediate/`, and `Saved/` are present locally.

## Recommended Next Steps

- Implement real save/load persistence behind `FAtlasSaveSystem` and `FAtlasLoadSystem`
- Expand `UAtlasGameInstanceSubsystem` with additional runtime services as the project grows
- Add actual input mapping contexts and bind them in `AAtlasPlayerController`
- Decide whether `AAtlasCharacter` or `AAtlasPawn` is the intended primary player avatar
- Create a project-owned startup map and stop relying on the engine template map
- Add gameplay assets, UI widgets, and Blueprint-facing data/content structure
- Wire `GameMode`, `PlayerController`, `GameState`, and `PlayerState` defaults into project settings or map overrides

## Summary

This project is currently an Atlas-structured UE 5.7 foundation with:

- a real multi-module runtime layout
- subsystem-based orchestration
- early save/load service registration
- gameplay framework base classes
- shared logging infrastructure

It is best described as an architectural base project rather than a feature-complete game slice.
