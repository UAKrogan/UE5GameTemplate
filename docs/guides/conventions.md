# Coding Conventions

Conventions enforced across all Atlas modules. Follow these for every contribution — reviews reject code that violates them.

---

## Class Naming

Standard Unreal prefixes, always combined with the `Atlas` project prefix:

| Prefix | Meaning | Example |
|---|---|---|
| `U` | `UObject`-derived | `UAtlasGameInstanceSubsystem` |
| `A` | `AActor`-derived | `AAtlasCharacter` |
| `F` | Plain struct / non-UObject class | `FAtlasSaveSystem`, `FAtlasWorldSnapshot` |
| `I` | Interface (abstract) | `IAtlasSaveSystem`, `IAtlasSavable` |
| `E` | Enum | `EAtlasSavePriority` |

- Every project type carries the `Atlas` prefix after the Unreal prefix.
- Pure C++ systems (implementations of `IAtlasSystem`) are `F`-prefixed and are **not** `UObject`s.

## Module API Macros

The export macro must match the module name exactly:

| Module | Macro |
|---|---|
| `Game` | `GAME_API` |
| `GameActors` | `GAMEACTORS_API` |
| `GameCore` | `GAMECORE_API` |
| `GameUI` | `GAMEUI_API` |
| `GameUtils` | `GAMEUTILS_API` |

Only types that are consumed outside the owning module get the API macro. Internal-only types stay unexported.

## Include Discipline

- Include what you use. Never rely on transitive includes.
- Public headers include only what their declarations require; prefer forward declarations for pointer/reference members.
- `PublicIncludePaths` / `PrivateIncludePaths` are set explicitly in every `.Build.cs` — never rely on include path pollution from other modules.
- `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs` in every `.Build.cs`.

## Pointer Rules

| Situation | Use |
|---|---|
| `UPROPERTY` owning reference | `TObjectPtr<T>` |
| Non-owning reference to a `UObject` | `TWeakObjectPtr<T>` |
| Async-loaded asset reference | `TSoftObjectPtr<T>` / `TSoftClassPtr<T>` |
| Non-`UObject` shared ownership (systems) | `TSharedPtr<T>` / `TSharedRef<T>`, `TSharedFromThis<T>` |
| Raw pointer | Only when lifetime is externally guaranteed — document the guarantee at the declaration |

Example of a documented raw pointer (see `FAtlasSaveSystem::OwningSubsystem`): the owning subsystem outlives the system and clears the pointer in `Shutdown()`.

## Ticking

Avoid `Tick` unless strictly necessary. Prefer:

- Delegates / events
- Timers (`FTimerManager`)
- Latent async tasks (`AsyncTask`, `UE::Tasks`)

If a class must tick, disable tick by default and enable only while needed.

## Logging

All logging goes through the `GameUtils` macros. Never use raw `UE_LOG` with ad-hoc categories in production code.

```cpp
ATLAS_LOG_CORE(Warning, "Save slot not found: %s", *SlotName);   // GameCore
ATLAS_LOG_ACTORS(Log, "Player controller initialized");          // GameActors
ATLAS_LOG_UI(Log, "Opened screen: %s", *ScreenName);             // GameUI
ATLAS_LOG_UTILS(Log, "Normalized string: %s", *Result);          // GameUtils
ATLAS_LOG_DEBUG(LogAtlasCore, "High-freq data: %d", Count);      // stripped in Shipping
```

Use `ATLAS_LOG_DEBUG` for high-frequency diagnostics — it compiles out of Shipping builds.

## Assertions

| Macro | Behavior | Use for |
|---|---|---|
| `ATLAS_CHECK(expr)` | Hard crash | Programmer errors only — conditions that can never be false in correct code |
| `ATLAS_ENSURE(expr)` | Logs and continues | Recoverable unexpected states |
| `ATLAS_ENSURE_MSG(expr, fmt, ...)` | Logs with context, continues | Same, when extra context helps triage |

Never use `ATLAS_CHECK` on conditions that depend on content, config, or user input.

## Blueprint Exposure

- Add `UFUNCTION(BlueprintCallable)` only for operations designers legitimately need to call. Do not blanket-expose internals.
- Use `BlueprintReadOnly` over `BlueprintReadWrite` unless mutation from Blueprint is a designed use case.
- Prefer `BlueprintNativeEvent` / `BlueprintImplementableEvent` hooks over exposing raw state.
- Pure C++ systems (`IAtlasSystem`) are never exposed to Blueprint directly — expose via a subsystem wrapper if needed.

## Asset Naming

| Prefix | Asset type |
|---|---|
| `DA_` | Data asset |
| `GA_` | Gameplay ability |
| `GE_` | Gameplay effect |
| `IA_` | Input action |
| `IMC_` | Input mapping context |
| `WBP_` | Widget Blueprint |
| `BP_` | Actor/object Blueprint |
| `GFD_` | Game Feature data |

Framework assets add the `Atlas` infix: `DA_Atlas_InputConfig`, `IMC_Atlas_Default`, `WBP_AtlasRoot`.

## Gameplay Tags

Hierarchical, rooted at `Atlas`:

```
Atlas.Category.Subcategory[.Leaf]
Atlas.Input.Ability.Jump
Atlas.UI.Layer.Menu
```

Native tags are declared in C++ (`GameCore/Public/Tags/AtlasGameplayTags.h`) — do not add framework tags via `DefaultGameplayTags.ini`.

## Comments

- Comments explain **why**, not **what**. If the code needs a "what" comment, rewrite the code.
- Document non-obvious constraints at the declaration site (threading rules, lifetime guarantees, initialization order).
- No multi-paragraph docstrings; a short block comment describing purpose and constraints is enough.

## Threading

- Systems and subsystems are game-thread-only unless explicitly documented otherwise.
- Async work (serialization, file I/O) marshals results back via `AsyncTask(ENamedThreads::GameThread, ...)` before touching game-thread state.
- State flags read by request entry points (e.g. `bSaveInProgress`) are game-thread-only; document this at the declaration.

## Module Boundaries

Dependencies flow one way only:

```
Game → GameActors, GameCore, GameUI, GameUtils
GameActors → GameUtils (+ GAS modules, EnhancedInput, AIModule, ModularGameplay, GameFeatures)
GameCore → GameUtils (+ GAS modules, DeveloperSettings; private UMG/Slate/Json)
GameUI → GameUtils (+ UMG, CommonUI, CommonInput, InputCore, DeveloperSettings, GameFeatures)
GameUtils → (no local deps)
```

Never introduce a dependency that violates this graph. Feature plugins and the `Game` host module are the only places allowed to depend on multiple local modules at once.
