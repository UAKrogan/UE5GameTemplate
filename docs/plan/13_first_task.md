# 13 — First Implementation Task

## Task: Phase 0 — Baseline Cleanup

This is a targeted cleanup of known risks identified in `00_repository_analysis.md`. It does not add any new functionality. It makes the project safer before new systems are built on top of it.

---

## Exact Goal

Fix the three concrete risks in the existing codebase:
1. Remove duplicate config entry in `DefaultEngine.ini`
2. Remove irrelevant Android File Server config from `DefaultEngine.ini`
3. Document the game-thread-only constraint on `bSaveInProgress` / `bLoadInProgress` in code (or switch to `TAtomic<bool>` if preferred)
4. Change `AAtlasGameMode` to extend `AGameMode` instead of `AGameModeBase`
5. Write `docs/guides/conventions.md`

---

## Files to Inspect Before Changing

1. `Config/DefaultEngine.ini` — locate duplicate `DefaultGraphicsRHI` and Android File Server block
2. `Source/GameCore/Private/Systems/Save/AtlasSaveSystem.cpp` — find `bSaveInProgress` usage
3. `Source/GameCore/Private/Systems/Load/AtlasLoadSystem.cpp` — find `bLoadInProgress` usage
4. `Source/GameActors/Public/GameMode/AtlasGameMode.h` — verify `AGameModeBase` base class
5. `Source/GameActors/Private/GameMode/AtlasGameMode.cpp` — verify no match-state-specific API is used (which would be absent on `AGameModeBase`)

---

## Files to Change

### `Config/DefaultEngine.ini`

**Remove** the duplicate `DefaultGraphicsRHI=DefaultGraphicsRHI_DX12` line (the second one is a duplicate).

**Remove** the entire `[/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings]` block — this is an Android tool setting that does not belong in a desktop PC template.

### `Source/GameActors/Public/GameMode/AtlasGameMode.h`

**Change** base class from `AGameModeBase` to `AGameMode`.

**Rationale:** `AGameMode` supports match state (`WaitingToStart`, `InProgress`, `WaitingPostMatch`), which is needed for any non-trivial game loop. `AGameModeBase` is too minimal. Both exist in `Engine` module which is already a dependency.

```cpp
// Before:
class GAMEACTORS_API AAtlasGameMode : public AGameModeBase

// After:
class GAMEACTORS_API AAtlasGameMode : public AGameMode
```

Include change: replace `#include "GameFramework/GameModeBase.h"` with `#include "GameFramework/GameMode.h"`.

### `Source/GameCore/Public/Systems/Save/AtlasSaveSystem.h`

Add a comment on `bSaveInProgress` explaining the constraint:

```cpp
// Only accessed on the game thread. Async callbacks must be dispatched via
// AsyncTask(ENamedThreads::GameThread, ...) before reading or writing this flag.
bool bSaveInProgress = false;
bool bShuttingDown = false;
```

### `Source/GameCore/Public/Systems/Load/AtlasLoadSystem.h`

Same comment on `bLoadInProgress`:

```cpp
// Only accessed on the game thread. Async callbacks must be dispatched via
// AsyncTask(ENamedThreads::GameThread, ...) before reading or writing this flag.
bool bLoadInProgress = false;
bool bShuttingDown = false;
```

### New file: `docs/guides/conventions.md`

Write the coding conventions guide as described in `11_documentation_plan.md`. Content source: already captured in `CLAUDE.md` under "UE C++ Conventions". Expand with all conventions from the plan.

---

## Proposed New Classes

None — this is a cleanup-only task.

---

## Build.cs Dependency Changes

None.

---

## Expected Compile Result

- Project builds with zero warnings or errors after the changes
- The only `AGameMode` API that `AAtlasGameMode` uses currently is `StartPlay()` and `InitializeGameMode()` — both exist on `AGameMode` as well
- No existing usage of `AGameModeBase`-specific API in the current thin implementation

---

## How to Test in Unreal Editor

1. **Generate project files** — after changing the base class, regenerate Visual Studio/Rider project files
2. **Build (Editor)** — verify clean compile
3. **Open editor** — verify no startup errors in Output Log
4. **PIE** — verify the default game mode works (open world map, PIE, check log for `[Actors] Game mode initialized`)
5. **Check Project Settings** — verify Android File Server block is gone (Settings → Platforms → Android — that section should no longer appear in the INI)
6. **Check engine ini duplication** — open `Config/DefaultEngine.ini` and verify only one `DefaultGraphicsRHI` line under the Windows settings section

---

## Rollback Strategy

All changes are:
- Config file edits (fully reversible via `git checkout Config/DefaultEngine.ini`)
- A single base class change that only affects `AAtlasGameMode` (no functionality existed on the base class beyond logging)
- Comment additions (no logic change)

If the `AGameMode` base class change causes unexpected issues (e.g., a required INI override references `AGameModeBase`), revert with:
```
git checkout Source/GameActors/Public/GameMode/AtlasGameMode.h
git checkout Source/GameActors/Private/GameMode/AtlasGameMode.cpp
```

No data assets or Blueprint classes depend on `AAtlasGameMode` yet, so there is no content breakage risk.
