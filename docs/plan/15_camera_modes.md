# 15 — Camera Mode Framework

All classes live in `GameActors`.

Replaces the "apply one static `UAtlasCameraConfig`" model with a **camera mode stack with blending** (Lyra-inspired, simplified), while keeping `UAtlasCameraConfig` as the data layer. Covers RPG first/third person switching, aim cameras, the RTS strategy camera, and the existing vehicle/mount camera pushes.

---

## Concepts

- **Camera mode** (`UAtlasCameraMode`): a blendable view policy — computes a desired view (pivot, rotation, arm length/offset, FOV) each update. Instanced UObject, one instance per mode class per component.
- **Mode stack** (inside `UAtlasCameraExtensionComponent`): top mode is authoritative; pushing a new mode blends in over `BlendTime`; fully-blended-out modes pop automatically.
- **Config-driven**: simple modes need no code — `UAtlasCameraMode_Config` reads a `UAtlasCameraConfig` asset, so today's data assets keep working.

## Key types

```cpp
// Computed per update by a mode; the component applies/blends the results.
USTRUCT()
struct FAtlasCameraView
{
    FVector  PivotLocation;      // usually pawn eye/root
    FRotator ControlRotation;    // desired view rotation
    float    ArmLength;          // 0 for first person
    FVector  SocketOffset;       // shoulder offset for TP
    float    FieldOfView;
    bool     bUsePawnControlRotation;
};

UCLASS(Abstract, Blueprintable)
class GAMEACTORS_API UAtlasCameraMode : public UObject
{
    // Identifies the mode for queries and input-config switching.
    UPROPERTY(EditDefaultsOnly, meta = (Categories = "Atlas.Camera.Mode"))
    FGameplayTag ModeTag;

    UPROPERTY(EditDefaultsOnly) float BlendTime = 0.3f;
    UPROPERTY(EditDefaultsOnly) TEnumAsByte<EViewTargetBlendFunction> BlendFunction;

    virtual void OnActivated(APawn* Pawn) {}
    virtual void OnDeactivated() {}
    virtual void UpdateView(APawn* Pawn, float DeltaTime, FAtlasCameraView& OutView) PURE_VIRTUAL(...);
};
```

## Built-in modes (framework-level, no game logic)

| Class | Behavior |
|---|---|
| `UAtlasCameraMode_Config` | Static view from a `UAtlasCameraConfig` (back-compat; default mode) |
| `UAtlasCameraMode_FirstPerson` | Arm length 0, pivot at eye height (socket name configurable, e.g. `head`), pawn control rotation, optional owner-mesh hiding hook |
| `UAtlasCameraMode_ThirdPerson` | Spring-arm view: arm length, shoulder offset, collision test (reuses `USpringArmComponent` probe settings), pawn control rotation |
| `UAtlasCameraMode_TopDown` | Fixed pitch, configurable height/zoom range, no control rotation — the RTS strategy camera (zoom driven externally by the strategy pawn, `17_selection_orders_rts.md`) |

## Component changes (`UAtlasCameraExtensionComponent`)

Extended, not replaced. Existing `PushCameraConfig`/`ClearPushedCameraConfig` become sugar over the stack (push/pop a `UAtlasCameraMode_Config`), so the vehicle/mount integration keeps working unchanged.

```cpp
// Stack API
void PushCameraMode(TSubclassOf<UAtlasCameraMode> ModeClass);
void PopCameraMode(TSubclassOf<UAtlasCameraMode> ModeClass);
const UAtlasCameraMode* GetTopCameraMode() const;
FGameplayTag GetActiveCameraModeTag() const;

// Delegate — input configs and animation listen for FP/TP switches
FAtlasCameraModeChanged OnCameraModeChanged; // (OldTag, NewTag)
```

- Ticks only while a blend is active (`SetComponentTickEnabled` on push/pop) — respects the "avoid Tick" convention.
- Blending: interpolate `FAtlasCameraView` fields between outgoing/incoming modes using the incoming mode's `BlendTime`/`BlendFunction`; write results into the pawn's spring arm + camera (same application path as today).
- Default stack bottom: `UAtlasCameraMode_Config` with `UAtlasPawnData::CameraConfig`.

## Pawn data & input integration

- `UAtlasPawnData` gains `TSubclassOf<UAtlasCameraMode> DefaultCameraMode` (falls back to `UAtlasCameraMode_Config` + `CameraConfig`).
- FP/TP toggle is an **ability**: a small framework ability `UAtlasCameraModeAbility` (grant via ability set, bound to e.g. `Atlas.Input.Ability.ToggleCamera`) that pushes/pops a configured mode class. Keeps camera switching data-driven and replication-friendly (mode is cosmetic; runs on owning client).
- Optional per-mode input configs: pawn data map `ModeTag -> UAtlasInputConfigData` applied by the input extension on `OnCameraModeChanged` (FP aiming sensitivities vs TP), reusing the suppress/apply machinery from the movement modes.

## New native tags (`GameCore` tags file)

```
Atlas.Camera.Mode.Default
Atlas.Camera.Mode.FirstPerson
Atlas.Camera.Mode.ThirdPerson
Atlas.Camera.Mode.TopDown
```

## Risks

- Blending math vs spring-arm collision interplay: keep collision resolution inside the TP mode (spring arm probe), blend the *desired* view, not the post-collision result.
- First person + full-body mesh needs game-side setup (hide head bone / separate arms mesh) — expose an `OnActivated` hook, do not solve it in the framework.

## How to test

1. PIE with a character whose pawn data sets a TP default mode; verify view matches the old static-config behavior.
2. Grant the camera toggle ability; press the bound key → smooth blend to FP and back; `OnCameraModeChanged` fires with the right tags.
3. Enter a vehicle (existing sample flow) → vehicle-pushed config still overrides; exit restores the character mode.
4. Instantiate `UAtlasCameraMode_TopDown` on a plain pawn → fixed-pitch overhead view, zoom clamps respected.
