# 11 — Documentation Plan

---

## Location

All project documentation lives in `docs/`. Plan files (this directory) are internal architecture documents. User-facing documentation lives in `docs/guides/`.

```
docs/
├── plan/                    ← architecture plan files (this directory)
│   ├── 00_repository_analysis.md
│   ├── 01_target_architecture.md
│   ├── ...
│   └── 13_first_task.md
└── guides/                  ← developer-facing guides (created incrementally)
    ├── README.md
    ├── architecture.md
    ├── modules.md
    ├── setup.md
    ├── common_ui.md
    ├── level_flow.md
    ├── gas.md
    ├── game_features.md
    ├── actors.md
    ├── conventions.md
    └── testing.md
```

---

## `docs/guides/README.md`

**Created in:** Phase 10 (template polish)  
**Content:**
- What this project is (reusable UE5 C++ template/framework)
- What it is NOT (a game, a demo, a content project)
- Engine version requirement
- How to open in editor
- How to generate project files (Visual Studio / Rider)
- How to build
- How to use the template as a base for a new game project
- Brief module summary with links to `modules.md`
- Link to full guides

---

## `docs/guides/architecture.md`

**Created in:** Phase 1 (after module boundaries are finalized)  
**Content:**
- Module dependency graph (diagram)
- Runtime lifecycle diagram (GameInstance → Subsystem → Systems)
- Layer diagram (GameUtils → GameCore/GameActors/GameUI → Game)
- Key design decisions and why

---

## `docs/guides/modules.md`

**Created in:** Phase 1  
**Content:**
- Detailed per-module responsibilities table
- What MUST be in each module
- What MUST NOT be in each module
- How to add a new module (checklist: Build.cs, module .h/.cpp, uproject entry)

---

## `docs/guides/setup.md`

**Created in:** Phase 2  
**Content:**
- Step-by-step new project setup using this template
- Configuring `UAtlasDeveloperSettings` (maps, pawn data, save slot)
- Setting `AssetManagerClass` in DefaultEngine.ini
- Setting game instance class
- Enabling `GameFeatures` and `ModularGameplay` plugins
- Creating a first gameplay map
- Setting default game mode, game state, player controller

---

## `docs/guides/common_ui.md`

**Created in:** Phase 3  
**Content:**
- CommonUI overview relevant to this template
- How screen push/pop works (`UAtlasUISubsystem`)
- Layer system (GameLayer, MenuLayer, ModalLayer, etc.)
- How to register a new screen (create `UAtlasScreenDefinition`, add to `UAtlasScreenRegistry`)
- How to create a new menu screen (subclass `UAtlasMenuWidget`, bind buttons, push sub-screens)
- Input mode handling per screen
- Gamepad navigation setup
- How to add HUD elements
- How to show a modal dialog
- How to show a notification
- Controller glyph system usage (`UAtlasGlyphWidget`)

---

## `docs/guides/level_flow.md`

**Created in:** Phase 4  
**Content:**
- Startup map → main menu → gameplay flow (diagram)
- How to request a level transition (`UAtlasLevelTransitionSubsystem::RequestTransition`)
- Pre-travel observers: how to register and what to do in them
- Post-load observers: how to register and what to do in them
- Loading screen: how to configure, how to show/hide manually
- How to add `AAtlasPostLoadTrigger` to a map
- `UAtlasDeveloperSettings` map configuration
- Seamless travel hook (override `PerformTravel()`)

---

## `docs/guides/gas.md`

**Created in:** Phase 5–6  
**Content:**
- GAS overview (brief, link to Epic docs for depth)
- How the Atlas template sets up GAS
- ASC ownership (PlayerState vs. Pawn — when and why)
- `UAtlasPawnData` setup: which pawn class, which ability sets, which input configs
- `UAtlasAbilitySet` authoring: add a new ability
- `UAtlasBaseGameplayAbility` subclassing
- `UAtlasBaseAttributeSet` subclassing
- Initialization order (player-controlled flow, AI flow)
- Input-to-ability binding via gameplay tags
- Effect strategy
- Replication considerations

---

## `docs/guides/game_features.md`

**Created in:** Phase 7–8  
**Content:**
- What Game Features plugins are in this template
- How to create a new Game Feature plugin (step by step)
- How to add abilities via feature plugin
- How to add input mappings via feature plugin
- How to add UI screens via feature plugin
- How to add HUD elements via feature plugin
- How to add actor components via `UGameFeatureAction_AddComponents`
- Activation/deactivation lifecycle
- How to enable/disable features in the editor
- Dependency direction rules (feature plugins must not be depended on by source modules)

---

## `docs/guides/actors.md`

**Created in:** Phase 5  
**Content:**
- Actor hierarchy diagram
- How to subclass `AAtlasCharacter` for a game project
- How to subclass `AAtlasPawn` for a non-humanoid pawn
- `UAtlasPawnExtensionComponent` — what it does, when it fires, extension points
- `UAtlasAbilityExtensionComponent` — granting ability sets
- `UAtlasInputExtensionComponent` — applying input contexts
- `UAtlasMovementExtensionComponent` — mode tracking
- Vehicle lifecycle (enter/exit via `UAtlasVehicleExtensionComponent`)
- Mount lifecycle (mount/dismount via `UAtlasMountExtensionComponent`)
- `UAtlasSavableComponent` — how to make an actor participate in save/load
- `IAtlasSavable` — how to implement `CaptureState` / `RestoreState`

---

## `docs/guides/conventions.md`

**Created in:** Phase 0 (baseline)  
**Content:**
- Class naming: prefix rules (U, A, F, I, E, Atlas)
- Module API macros
- Include discipline (no transitive includes)
- `TObjectPtr` vs `TWeakObjectPtr` vs raw pointer rules
- No tick unless necessary
- Logging: which macro to use per module
- Assertion usage: `ATLAS_CHECK` vs `ATLAS_ENSURE` vs `ATLAS_ENSURE_MSG`
- Build.cs: always explicit public/private includes
- Blueprint exposure guidelines (when to add UFUNCTION, when not to)
- Data asset naming conventions (`DA_*`, `GA_*`, `IMC_*`, etc.)
- Gameplay tag naming convention (`Atlas.Category.Subcategory`)
- Comment policy: WHY not WHAT; no multi-paragraph docstrings

---

## `docs/guides/testing.md`

**Created in:** Phase 10  
**Content:**
- How to test Atlas systems in Play-In-Editor (PIE)
- How to test level transitions (PIE + standalone)
- How to test save/load (how to inspect save files, how to force-load a save)
- How to test Game Feature activation/deactivation in the editor
- How to test gamepad input in PIE
- Unreal Automation framework: where to put tests, how to run
- How to validate GAS ability grants (use Gameplay Debugger: `'` key)
- How to validate gameplay tags (GameplayTag Debugger in editor)
- Known PIE-vs-standalone differences to watch for

---

## Documentation Authoring Rules

- Guides are written in Markdown and rendered on GitHub
- Code examples use C++ with Atlas naming conventions
- Diagrams are ASCII (no external image dependencies)
- Guides reference class names with full module qualification (`GameCore::UAtlasGameFlowSubsystem`) on first mention only
- Guides never repeat content from CLAUDE.md — CLAUDE.md is for Claude Code only
- Update guides in the same PR as the feature they document
