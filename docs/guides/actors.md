# Actors and Extension Components

The `GameActors` hierarchy and the component contract that keeps base classes thin.

## Hierarchy

```
APawn
└── AAtlasPawn                 (generic pawn, GAS-ready)
    ├── AAtlasVehiclePawn      (multi-seat vehicle; physics added per game)
    └── AAtlasMountPawn        (single-rider mount)
ACharacter
└── AAtlasCharacter            (humanoid; keeps ACharacter for CharacterMovement + animation)
AController
├── AAtlasPlayerController     (owns UAtlasInputExtensionComponent)
└── AAtlasAIController         (initializes pawn-owned ASC on possess)
AGameMode      └── AAtlasGameMode       (match state; pawn class from pawn data)
AGameState     └── AAtlasGameState
APlayerState   └── AAtlasPlayerState    (owns the player's ASC)
```

Design rule: **components over god classes**. Base actors provide hooks and delegate to extension components; a pawn's capabilities come from its `UAtlasPawnData` asset, not hardcoded logic.

All base actors participate in the Modular Gameplay receiver pattern (`AddGameFrameworkComponentReceiver` in `PreInitializeComponents`, `GameActorReady` in `BeginPlay`, `RemoveReceiver` in `EndPlay`) so Game Feature plugins can extend them at runtime.

## Subclassing for a game

- Humanoid player/NPC: subclass `AAtlasCharacter`. Override `InitializeCharacter()` for game setup; don't reimplement possession plumbing.
- Non-humanoid pawn (turret, drone, camera pawn): subclass `AAtlasPawn`, override `InitializePawn()`.
- Both already carry `PawnExtComp` + `AbilityExtComp`; the character adds `MovementExtComp`.

## UAtlasPawnExtensionComponent

The GAS coordination hub on every pawn.

- `SetPawnData()` — assign before the ability system initializes (defaults can be set on the archetype).
- `HandleControllerChanged()` — called from `PossessedBy` / `UnPossessed` / `OnRep_Controller` / `OnRep_PlayerState`; binds to the PlayerState ASC when one exists.
- `InitializeAbilitySystem(ASC, OwnerActor)` — idempotent; runs `InitAbilityActorInfo`, grants pawn data ability sets on the authority, then fires `OnAbilitySystemInitialized`.
- `UninitializeAbilitySystem()` — revokes granted sets, releases the avatar, fires `OnAbilitySystemUninitialized`.

## UAtlasAbilityExtensionComponent

Provides the pawn-owned ASC for AI/standalone pawns (`GetOrCreateAbilitySystemComponent()`) and grant/revoke wrappers. Player pawns never use it for ownership — their ASC lives on the PlayerState.

## UAtlasInputExtensionComponent

Lives on `AAtlasPlayerController`. `SetupPlayerInputComponent` (which runs on the owning client, unlike `OnPossess`) forwards to `InitializeForPawn(Pawn, InputComponent)`, which:

1. Loads the pawn data's `UAtlasInputConfigData` assets.
2. Adds each mapping context to the local player's Enhanced Input subsystem.
3. Binds each ability input action on the pawn's `UEnhancedInputComponent` — `Started` → `AbilityInputTagPressed(Tag)`, `Completed` → `AbilityInputTagReleased(Tag)`.

Bindings die with the pawn's input component; mapping contexts are removed in `OnUnPossess`. `ApplyConfig`/`RemoveConfig` support runtime configs from Game Feature actions.

## UAtlasMovementExtensionComponent

The authority on a pawn's high-level movement mode (`EAtlasMovementMode`: `None/Walking/Falling/Swimming/Flying/Custom/InVehicle/Mounted`). It routes transitions and broadcasts `OnMovementModeChanged`; it does not drive physics.

- **Engine sync** — for `ACharacter` owners it mirrors `UCharacterMovementComponent` mode changes (walk/fall/swim/fly/custom) automatically.
- **Attachment transitions** — `EnterVehicle`/`ExitVehicle` and `MountRider`/`Dismount` save the current mode, disable the pawn's own movement component, attach/detach the pawn to the seat or rider socket, and restore the saved mode on exit. Authority only; called by the vehicle/mount extension components, not directly.
- **Replication** — the mode replicates with a rep-notify that rebroadcasts on clients; attachment replicates natively with pawn movement.
- `RequestMovementModeChange()` remains for self-propelled modes and rejects `InVehicle`/`Mounted` (those carry attachment side effects).

The input extension listens to mode changes: while `InVehicle`/`Mounted` the pawn data's default mapping contexts are suppressed (contexts only — ability bindings stay put, so resuming never double-binds).

## Vehicles — AAtlasVehiclePawn + UAtlasVehicleExtensionComponent

`AAtlasVehiclePawn` deliberately ships no vehicle physics; game projects add their movement component (Chaos, custom) and it is resolved via `GetVehicleMovementComponent()`. Seats attach at `Seat_<Index>` sockets (`GetSeatSocketName`/`GetSeatAttachComponent` are overridable, e.g. for skeletal meshes).

`UAtlasVehicleExtensionComponent` owns the lifecycle:

```cpp
VehicleExt->TryEnterVehicle(Passenger);        // -1 = first free seat
VehicleExt->ExitVehicle(Passenger);
VehicleExt->IsSeatOccupied(0);  VehicleExt->GetPassengerCount();
```

On enter it drives the passenger's movement extension, records the seat, and applies the optional `PassengerInputConfig` (drive controls) and `PassengerCameraConfig` to the passenger; exit reverses everything. The vehicle's `OnPassengerEntered/Exited` virtuals (+ Blueprint events) and the component delegates fire on both.

## Mounts — AAtlasMountPawn + UAtlasMountExtensionComponent

Single-rider variant of the same pattern: `TryMount(Rider)` / `Dismount()`, rider attaches at `GetRiderAttachSocket()` (default `RiderSocket`), `CanBeRidden()` is overridable for taming/ownership rules, and optional `RiderInputConfig`/`RiderCameraConfig` apply while mounted.

## Camera — UAtlasCameraConfig + UAtlasCameraExtensionComponent

`UAtlasCameraConfig` (data asset) holds spring arm and camera settings (arm length, control-rotation flags, FOV, blend hints). The optional `UAtlasCameraExtensionComponent` applies the active config to the pawn's spring arm/camera components: the default comes from `UAtlasPawnData::CameraConfig`; vehicles and mounts `PushCameraConfig()` their own while the pawn is attached and the default is restored on exit. Override `GetActiveCameraConfig()` for extra layers (aim camera, photo mode).

## Save participation

- Add `UAtlasSavableComponent` to give the actor a stable `FGuid ActorId`.
- Implement `IAtlasSavable` (`CaptureState` / `RestoreState`) on the actor or its components.
- `UAtlasGASSaveAdapter` — drop-in component capturing/restoring ASC attributes and active effects.
- `UAtlasInventorySaveAdapter` — stub for future inventory systems.

The save collector picks up any actor with a savable component automatically; the load system resolves actors by GUID, spawning missing ones from their recorded class.
