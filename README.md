# UE5GameTemplate (Atlas)

A reusable **Unreal Engine 5.7 C++ framework** for building games. It is not a game — it ships no gameplay mechanics, authored widgets, or production maps. It provides the runtime skeleton a game builds on, with strict module boundaries and data-driven configuration.

## What's inside

- **Module architecture** — five one-way-dependency modules (`Game` host, `GameCore`, `GameActors`, `GameUI`, `GameUtils`)
- **Core runtime** — `UAtlasGameInstanceSubsystem` service locator over pure C++ `IAtlasSystem` systems; `UAtlasAssetManager`; `UAtlasDeveloperSettings`; native gameplay tags
- **Game flow** — `UAtlasGameFlowSubsystem` state machine, `UAtlasLevelTransitionSubsystem` travel lifecycle with pre/post hooks, travel-surviving loading screen
- **GAS-ready actors** — PlayerState-owned ASC for players, pawn-owned for AI; extension components coordinate initialization; abilities/effects/attributes granted from `UAtlasAbilitySet` data assets
- **Vehicles & mounts** — replicated high-level movement modes, seat/rider lifecycle with enter/exit attachment, per-mode input contexts and camera configs (physics left to the game project)
- **Enhanced Input → abilities** — tag-based activation via `UAtlasInputConfigData`; input never references ability classes
- **Common UI** — layered root widget (Game/Menu/Modal/Notification/Loading), screens pushed by ID through a registry, input mode owned by the stack, data-driven controller glyphs
- **Game Features** — all base actors are Modular Gameplay receivers; custom actions add abilities, input configs, screens, and HUD elements from feature plugins
- **Save/load** — async chunked binary pipeline with priority scheduler, 5-slot autosave ring, GUID-based actor resolution, version migrations, and a JSON slot manifest

## Getting started

Requirements: UE 5.7, Visual Studio 2022+ or Rider.

1. Right-click `Game.uproject` → *Generate Visual Studio project files* (or open in Rider).
2. Build `GameEditor` (Development | Win64) and open the project.
3. Follow [docs/guides/setup.md](docs/guides/setup.md) to configure maps, pawn data, and UI for your game.

## Documentation

Start at [docs/guides/README.md](docs/guides/README.md):
architecture, modules, setup, Common UI, level flow, GAS, actors, Game Features, conventions, testing.

All planning docs are implemented and removed (preserved in git history); [docs/plan/README.md](docs/plan/README.md) tracks the few small remnants.

## Layout

```
Source/
  Game/        thin host (UAtlasGameInstance)
  GameCore/    runtime systems, flow, save/load, settings, tags
  GameActors/  actor bases, extension components, GAS, input data
  GameUI/      UI subsystem, widgets, screen registry, glyphs
  GameUtils/   logging + assertion macros
Plugins/GameFeatures/   sample feature plugins ([TEMPLATE EXAMPLE], deletable)
Config/                 project config (Atlas settings in DefaultGame.ini)
docs/                   guides + open future-work designs
```

## Still content-free by design

Maps, widget Blueprints, input assets, and Game Feature data assets must be authored in the editor per project — the sample plugin READMEs and `docs/guides/setup.md` list exactly what to create.
