# Atlas Framework Guides

**UE5GameTemplate** is a reusable Unreal Engine 5.7 C++ framework (the `Atlas` prefix) intended as a base for different games.

It is **not** a game, a demo, or a content project — it ships no gameplay mechanics, no authored widgets, and no production maps. It provides the runtime skeleton a game builds on: module boundaries, subsystem orchestration, GAS-ready actors, a Common UI layer stack, level flow, and a save/load pipeline.

## Requirements

- Unreal Engine **5.7**
- Visual Studio 2022+ or Rider (Windows)

## Getting started

1. Clone the repository.
2. Right-click `Game.uproject` → *Generate Visual Studio project files* (or open the uproject directly in Rider).
3. Build the `GameEditor` target (Development | Win64), or from a terminal:
   ```
   <UE5_ROOT>\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe GameEditor Win64 Development "<repo>\Game.uproject"
   ```
4. Open `Game.uproject` in the editor.

To use the template as a base for a new game, see [setup.md](setup.md).

## Modules at a glance

| Module | Purpose |
|---|---|
| `Game` | Thin host; `UAtlasGameInstance` bootstrap only |
| `GameCore` | Runtime systems, game flow, level transitions, save/load, settings, asset manager, tags |
| `GameActors` | GAS-ready actor base classes, extension components, pawn/ability/input data assets |
| `GameUI` | Common UI layer stack, screen registry, base widgets, glyphs |
| `GameUtils` | Log categories, logging/assertion macros |

Full details: [modules.md](modules.md) and [architecture.md](architecture.md).

## Guides

- [architecture.md](architecture.md) — module graph, runtime lifecycle, design decisions
- [modules.md](modules.md) — per-module responsibilities and the new-module checklist
- [setup.md](setup.md) — using the template for a new game project
- [common_ui.md](common_ui.md) — screens, layers, modals, notifications, glyphs
- [level_flow.md](level_flow.md) — game flow states, level transitions, loading screens
- [gas.md](gas.md) — ability system setup, ownership, input binding
- [actors.md](actors.md) — actor hierarchy and extension components
- [game_features.md](game_features.md) — Game Feature plugins and custom actions
- [conventions.md](conventions.md) — coding conventions
- [testing.md](testing.md) — how to test each system
