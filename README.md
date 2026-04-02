# Game

`Game` is an Unreal Engine 5.7 C++ starter project with a modular source layout and modern rendering settings enabled. The repository currently contains a clean code foundation, project configuration, the default Open World template map, and local editor/build artifacts. There is no implemented gameplay feature set yet.

## Current Project State

- Engine association: `5.7`
- Project file: `Game.uproject`
- Solution file: `Game.sln`
- Registered project module in `Game.uproject`: `Game`
- Additional local code modules compiled through dependencies: `GameCore`, `GameSystems`, `GameUI`, `GameUtils`
- Enabled project plugin: `ModelingToolsEditorMode` for editor targets
- Local developer plugin present: `Plugins/Developer/RiderLink`

Important detail: only the `Game` module is declared in `Game.uproject`. The other modules exist under `Source/` and are referenced by `Game.Build.cs`, so they are part of the local codebase and build graph, but not separately listed in the project descriptor.

## Source Layout

```text
Source/
|-- Game/                   # Primary runtime module
|-- GameCore/               # Shared core code scaffold
|-- GameSystems/            # Gameplay/system code scaffold
|-- GameUI/                 # UI code scaffold
|-- GameUtils/              # Shared utility code scaffold
|-- Game.Target.cs          # Runtime target
`-- GameEditor.Target.cs    # Editor target
```

Each module currently follows the standard Unreal layout:

```text
<Module>/
|-- Public/
|-- Private/
`-- <Module>.Build.cs
```

## Module Status

### `Game`

- Declares the primary game module with `IMPLEMENT_PRIMARY_GAME_MODULE`
- Depends on `EnhancedInput` plus all four local modules
- Logs startup and shutdown through `UE_LOG`
- Contains commented-out explicit module loads, which are not currently used

### `GameCore`

- Minimal runtime module scaffold
- Depends on `Core`, `CoreUObject`, and `Engine`
- Only logs startup and shutdown

### `GameSystems`

- Minimal runtime module scaffold
- Depends on `GameUtils`
- Only logs startup and shutdown

### `GameUI`

- Minimal runtime module scaffold
- Publicly depends on `UMG` and `GameUtils`
- Privately depends on `Slate` and `SlateCore`
- Only logs startup and shutdown

### `GameUtils`

- Minimal runtime module scaffold
- Lightweight shared dependency surface
- Only logs startup and shutdown

At the moment, the codebase is architecture-first: module boundaries exist, but there are no gameplay classes, widgets, subsystems, actors, or utility implementations beyond module bootstrap code.

## Configuration Snapshot

`Config/DefaultEngine.ini` currently sets:

- Default map: `/Engine/Maps/Templates/OpenWorld`
- Desktop hardware targeting with maximum graphics profile
- DirectX 12 as the default Windows RHI
- Shader Model 6 for D3D12
- Static lighting disabled
- Mesh distance fields enabled
- Dynamic GI and reflections enabled
- Ray tracing enabled
- Substrate enabled
- Virtual shadow maps enabled

`Config/DefaultInput.ini` currently sets:

- `EnhancedInput` classes as the default player input and input component
- Standard mouse and gamepad axis tuning from the template project
- No custom action or axis mappings beyond template defaults

`Config/DefaultGame.ini` currently contains:

- `CommonUI` accept key handling
- The generated Unreal `ProjectID`

## Content Status

`Content/` currently contains only:

- `Collections/`
- `Developers/`

There are no game-facing assets, maps, Blueprints, widgets, or data assets checked into the project root content folders yet.

## Generated and Local Folders Present

This project directory currently includes Unreal-generated or machine-local folders:

- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- `.vs/`
- `.idea/`

These are useful locally, but they generally should not be treated as source-of-truth project content.

## Open The Project

### Unreal Editor

Open `Game.uproject` with Unreal Engine 5.7.

### IDE

- Visual Studio: open `Game.sln`
- Rider: open the folder or `Game.sln`

## Repository Hygiene Notes

- There is currently no `.gitignore` file in this folder.
- This folder is not currently initialized as a Git repository.

If this project will be version-controlled, add a standard Unreal `.gitignore` and exclude generated directories such as `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.vs/`, and optionally `.idea/`.

## Recommended Next Steps

- Add actual gameplay classes to `GameCore` and `GameSystems`
- Add reusable helpers to `GameUtils`
- Add widgets, UI flow, and input-driven screens to `GameUI`
- Replace the template startup map with a project-specific map
- Add real assets under `Content/` outside developer-only folders
- Decide whether the extra modules should remain implicit via build dependencies or also be declared explicitly in project metadata as the architecture grows

## Summary

The project is currently a clean Unreal Engine 5.7 modular starter: code modules are separated well, rendering features are configured aggressively for modern hardware, and input is set up for `EnhancedInput`. The missing pieces are gameplay code, real content, and repository hygiene.
