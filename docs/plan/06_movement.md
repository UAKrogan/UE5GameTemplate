# 06 — Movement Architecture

All classes live in `GameActors` unless stated otherwise.

---

## Philosophy

- Movement is component-driven. The base actor classes do not hard-code movement logic.
- `UAtlasMovementExtensionComponent` is the mode router and the authority on the current movement state.
- Each movement mode (character, vehicle, mounted, AI) is a separate concern handled by the appropriate UE component plus Atlas extension contracts.
- The Atlas layer defines contracts (interfaces, enums, delegates), not concrete physics implementations.
- Multiplayer replication uses UE's native movement component replication. No custom networking is added in this layer.

---

## `EAtlasMovementMode`

```cpp
UENUM(BlueprintType)
enum class EAtlasMovementMode : uint8
{
    None,
    Walking,
    Falling,
    Swimming,
    Flying,
    Custom,          // game-specific extension
    InVehicle,       // pawn is a passenger, not self-propelling
    Mounted,         // pawn is riding a mount
};
```

---

## `UAtlasMovementExtensionComponent`

**Module:** `GameActors`
**Extends:** `UActorComponent`
**Responsibility:** Tracks the authoritative movement mode, routes mode transitions, broadcasts change events. Does not directly drive physics.

**Key API:**
```cpp
EAtlasMovementMode GetCurrentMode() const;
void EnterVehicle(AAtlasVehiclePawn* Vehicle, int32 SeatIndex);
void ExitVehicle();
void MountRider(AAtlasMountPawn* Mount);
void Dismount();

// Delegates
DECLARE_MULTICAST_DELEGATE_TwoParams(FAtlasMovementModeChanged,
    EAtlasMovementMode OldMode, EAtlasMovementMode NewMode);

FAtlasMovementModeChanged OnMovementModeChanged;
```

**On `EnterVehicle()`:**
1. Saves current mode
2. Sets mode to `InVehicle`
3. Disables native `UCharacterMovementComponent` (sets movement mode to None)
4. Attaches pawn to vehicle socket
5. Broadcasts `OnMovementModeChanged`
6. Notifies `UAtlasInputExtensionComponent` to switch input mapping context

**On `ExitVehicle()`:**
1. Detaches pawn from vehicle
2. Restores saved movement mode
3. Re-enables `UCharacterMovementComponent`
4. Broadcasts `OnMovementModeChanged`
5. Restores original input mapping context

---

## Humanoid Character Movement

**Component:** `UCharacterMovementComponent` (UE native, on `AAtlasCharacter`)
**Atlas additions:**
- `UAtlasMovementExtensionComponent` monitors `CharacterMovement->MovementMode` changes and syncs to `EAtlasMovementMode`
- No custom movement component needed for humanoid movement
- Game projects subclass `UCharacterMovementComponent` for custom movement modes (climbing, grappling, etc.)

**Extension point:**
```cpp
// In AAtlasCharacter:
virtual UCharacterMovementComponent* CreateCharacterMovementComponent();
```
Override to return a game-specific movement component subclass.

---

## Generic Pawn Movement

**Component:** `UFloatingPawnMovement` (UE native) OR `UPawnMovementComponent` subclass on `AAtlasPawn`
**Atlas additions:** `UAtlasMovementExtensionComponent` for mode tracking only.

`AAtlasPawn` does not use `UCharacterMovementComponent`. It exposes:
```cpp
// In AAtlasPawn:
virtual UPawnMovementComponent* CreateMovementComponent();
```
Default: `UFloatingPawnMovement`. Game projects override for custom simple movement.

---

## Vehicle Movement

**Actor:** `AAtlasVehiclePawn`
**Movement component:** Not defined by the Atlas layer. Game projects add their preferred vehicle movement component (e.g., `UChaosVehicleMovementComponent` for Chaos physics vehicles, or a custom component).

**Atlas extension contract:**
```cpp
// In AAtlasVehiclePawn:
virtual UPawnMovementComponent* GetVehicleMovementComponent() const;
virtual void OnPassengerEntered(APawn* Passenger, int32 SeatIndex);
virtual void OnPassengerExited(APawn* Passenger, int32 SeatIndex);
```

`UAtlasVehicleExtensionComponent` on the vehicle actor handles the enter/exit lifecycle:
1. Passenger calls `TryEnterVehicle()`
2. Component checks seat availability
3. Calls `Passenger->MovementExtComp->EnterVehicle(this, SeatIndex)`
4. Calls `OnPassengerEntered()` virtual on the vehicle
5. Routing complete — vehicle's own movement component drives the vehicle

---

## Mount / Riding Movement

**Actor:** `AAtlasMountPawn`
**Movement component:** Same as `AAtlasPawn` — determined by the game project.

**Atlas mount contract:**
```cpp
// In AAtlasMountPawn:
virtual bool CanBeRidden() const;
virtual FName GetRiderAttachSocket() const;  // default: "RiderSocket"
virtual void OnRiderMounted(APawn* Rider);
virtual void OnRiderDismounted(APawn* Rider);
```

**Mount lifecycle:**
1. Rider calls `UAtlasMountExtensionComponent::TryMount(Mount)`
2. Mount checks `CanBeRidden()`
3. Rider's movement extension enters `Mounted` mode (disables rider's own movement component)
4. Rider is attached to mount's `RiderAttachSocket`
5. Input routing: rider's `UAtlasInputExtensionComponent` switches to a "mounted" input context
6. Mount's movement component drives both mount and rider
7. On `Dismount()`: rider detaches, rider movement restored, mount's `OnRiderDismounted()` called

---

## AI Movement

**Component:** `UNavMovementComponent` or `UCharacterMovementComponent` (on AI-controlled pawns) + UE's navigation system.
**Atlas additions:**
- `AAtlasAIController` uses `UAIMovementComponent` if present, or delegates to `UCharacterMovementComponent` for humanoid AI
- No custom AI movement in the Atlas layer
- Game projects plug in BehaviorTree/EQS for decision-making

**`UAtlasMovementExtensionComponent` on AI pawns:** Tracks mode in case an AI pawn enters a vehicle or gets mounted (e.g., enemy on horseback), using the same enter/exit lifecycle as player pawns.

---

## Possession Switching

When a player switches possession (e.g., exits a vehicle and re-possesses their character):

1. `AAtlasPlayerController::UnPossess()` fires
   - Current pawn's `UAtlasMovementExtensionComponent` cleans up (exit vehicle/mount if needed)
   - `UAtlasInputExtensionComponent` removes input mapping contexts
2. `AAtlasPlayerController::Possess(NewPawn)` fires
   - `NewPawn->PawnExtComp->HandleControllerChanged()`
   - `UAtlasInputExtensionComponent` applies new pawn's input contexts
   - Camera snaps to new pawn's camera (or blends if camera blend is configured)

---

## Camera and Control Mode Switching

`UAtlasCameraExtensionComponent` *(new, optional, part of GameActors)*:
- Tracks the current camera configuration data asset (`UAtlasCameraConfig`)
- On possession: applies spring arm / camera settings from pawn data
- On mount: transitions to mount camera config
- On vehicle: transitions to vehicle camera config
- Game projects extend by overriding `GetActiveCameraConfig()`

`UAtlasCameraConfig` — data asset:
```cpp
float SpringArmLength;
FRotator DefaultRotation;
bool bUsePawnControlRotation;
bool bInheritPitch, bInheritYaw, bInheritRoll;
float FOV;
float BlendTime;
TEnumAsByte<EViewTargetBlendFunction> BlendFunction;
```

---

## Input Routing During Movement Mode Changes

| Mode | Active Input Context |
|---|---|
| Walking/Falling/Swimming | Character input context from pawn data |
| InVehicle (driver) | Vehicle driver input context |
| InVehicle (passenger) | Passenger-only input context (no drive inputs) |
| Mounted (rider) | Mount rider input context |
| Mounted (mount has own AI) | Mount still uses its own AI, rider context limited to dismount |

Input context switching is handled by `UAtlasInputExtensionComponent` in response to `UAtlasMovementExtensionComponent::OnMovementModeChanged`.

---

## Replication Considerations

- `UCharacterMovementComponent` replication is handled entirely by UE — no Atlas additions needed.
- `EAtlasMovementMode` should be replicated as a `UPROPERTY(ReplicatedUsing=...)` in `UAtlasMovementExtensionComponent` so clients can update their visual state correctly.
- Vehicle/mount attachment is replicated via `AActor::AttachToComponent()` which UE replicates natively when `bReplicateMovement` is true.
- On client, `OnRep_AttachmentReplication()` should trigger `UAtlasMovementExtensionComponent` to sync its local state.

```cpp
// In UAtlasMovementExtensionComponent:
UPROPERTY(ReplicatedUsing=OnRep_MovementMode)
EAtlasMovementMode CurrentMovementMode;

UFUNCTION()
void OnRep_MovementMode();
```
