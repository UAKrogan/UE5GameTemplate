# 17 — Selection, Orders, and the RTS Player Shell

All classes live in `GameActors`. Three layers: selectable units, a per-player selection service, and a tag-based order pipeline that routes commands into unit ASCs. The strategy pawn/controller complete the RTS player side. Selection and orders are deliberately genre-agnostic — RPG companion commands and lock-on targeting use the same seams.

---

## Selection

### `UAtlasSelectableComponent` (on units/actors)

- Opt-in marker + state: `bIsSelected`, `OnSelectionChanged(bool)` (cosmetic hooks: decals, outlines — game side).
- `CanBeSelectedBy(const AController*) const`: default = same team (via `UAtlasTeamStatics`); overridable for spectators/neutral buildings.
- Optional `SelectionPriority` and `FGameplayTag UnitTypeTag` (e.g. `Atlas.Unit.Worker` — tags declared by game/feature, not the framework) for grouping and UI.

### `UAtlasSelectionSubsystem` (`ULocalPlayerSubsystem`)

Selection is per-player, local-only state (server validates orders separately — selection itself never replicates).

```cpp
void SelectActor(AActor* Actor, bool bAddToSelection);
void SelectActors(const TArray<AActor*>& Actors, bool bAddToSelection);   // marquee result
void Deselect(AActor* Actor);
void ClearSelection();
const TArray<TWeakObjectPtr<AActor>>& GetSelection() const;
AActor* GetPrimarySelection() const;                                      // first valid

// Control groups (RTS 1-9): snapshot/recall of selections
void AssignControlGroup(int32 Index);
void RecallControlGroup(int32 Index, bool bAdditive);

FAtlasSelectionChanged OnSelectionChanged;                                // UI binds unit portraits etc.
```

- Marquee support: the subsystem exposes `SelectInScreenRect(FVector2D Start, FVector2D End, bool bAdditive)` implementing frustum-select over selectable components (project bounds to screen); drawing the rectangle is UI/game-side.
- RPG usage: single-select as lock-on target (`GetPrimarySelection`), companions in control group 1.

## Orders

Orders are **data commands routed as gameplay events** — the framework never defines what "Move" does; a unit's granted abilities do.

```cpp
USTRUCT(BlueprintType)
struct FAtlasOrder
{
    // Identifies the order (Atlas.Order.Move / .Attack / .Stop / game-defined).
    UPROPERTY(...) FGameplayTag OrderTag;
    UPROPERTY(...) FVector      TargetLocation = FVector::ZeroVector;
    UPROPERTY(...) TWeakObjectPtr<AActor> TargetActor;
    UPROPERTY(...) bool         bQueued = false;   // shift-queue support flag
};
```

### `UAtlasOrderComponent` (on units)

- `IssueOrder(const FAtlasOrder&)` (authority): validates (team attitude for targeted orders via `UAtlasTeamStatics`), then sends a **gameplay event** to the unit's ASC — `FGameplayEventData{ EventTag = OrderTag, Target/OptionalObject/Location }`. Abilities granted from the unit's ability sets are triggered by the event tag (`GA_Move` triggers on `Atlas.Order.Move`, uses AIController move; framework ships no concrete order abilities).
- Falls back to a Blueprint-assignable `OnOrderReceived` delegate for units without an ASC.
- Optional simple queue (`bQueued`): pending orders drained when the active ability ends (listens to the ASC).

### Issuing side — `UAtlasOrderStatics`

`IssueOrderToSelection(APlayerController*, const FAtlasOrder&)`: iterates the local selection subsystem, server-RPC path through the player controller (documented seam: `AAtlasStrategyPlayerController::ServerIssueOrder`), calls each unit's order component on the authority.

### New native tags (`GameCore`)

```
Atlas.Order.Move
Atlas.Order.Attack
Atlas.Order.Stop
Atlas.Input.Native.Select        (LMB)
Atlas.Input.Native.Command       (RMB context command)
Atlas.Input.Native.CameraPan / CameraZoom / CameraRotate
```

## RTS player shell

### `AAtlasStrategyPawn` (extends `AAtlasPawn`)

- Free camera pawn: `UFloatingPawnMovement` + spring arm + camera, driven by `UAtlasCameraMode_TopDown` (`15_camera_modes.md`).
- Pan (keys + optional edge scroll, configurable margins/speed), zoom (arm-length range with pitch curve), optional rotation; all input via a normal `UAtlasInputConfigData` using the `Atlas.Input.Native.Camera*` actions bound natively in the pawn (native input, not GAS — camera movement is not an ability).
- Height follow: optional ground trace to keep constant height over terrain.

### `AAtlasStrategyPlayerController` (extends `AAtlasPlayerController`)

- `bShowMouseCursor` default true; input mode `GameAndMenu`.
- Click routing: LMB press/drag → selection subsystem (single vs `SelectInScreenRect`); RMB → builds a context `FAtlasOrder` from the cursor hit (hostile actor under cursor + team check → `Atlas.Order.Attack`, ground → `Atlas.Order.Move`) and calls `IssueOrderToSelection`; shift modifies `bQueued`.
- `ServerIssueOrder(FAtlasOrder, TArray<AActor*>)` RPC — the authority validation point.
- Context-order resolution is virtual (`ResolveContextOrder(HitResult) -> FAtlasOrder`) so games change RMB semantics without touching selection plumbing.

## Module/dependency impact

None new — GAS, EnhancedInput, AIModule already in `GameActors` deps.

## Risks

- Marquee frustum-select needs careful screen-projection of bounds; start with actor-location-in-rect (cheap, good enough for the template) and document the upgrade path.
- Order → ability event assumes units have ASCs with event-triggered abilities; the non-ASC delegate fallback keeps simple units working.
- Multiplayer: selection local, orders server-validated — the RPC seam must be in from the start even if samples are single-player.

## How to test

1. Possess a strategy pawn in a test map: pan/zoom/edge-scroll work; camera stays above terrain.
2. Place pawns with selectable + team components: LMB selects own unit, cannot select enemy; drag-select picks multiple; control group assign/recall works.
3. Give a unit an event-triggered move ability: RMB on ground → unit moves; RMB on a hostile → attack event tag arrives; shift-RMB queues.
4. RPG check: single-select a companion, issue `Atlas.Order.Move` from a radial menu stub → same pipeline works without the strategy controller.
