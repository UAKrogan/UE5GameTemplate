# 05 — Gameplay Actor Architecture

All classes live in `GameActors` unless stated otherwise.

---

## Design Philosophy

- Prefer **components over god classes**. Base actor classes provide hooks and delegation, not direct implementation.
- A pawn's capabilities are defined by its **pawn data asset** (`UAtlasPawnData`), not by hardcoded logic.
- GAS initialization is coordinated through `UAtlasPawnExtensionComponent`, not scattered across actors.
- Input binding to abilities is handled by `UAtlasInputExtensionComponent`, not by the player controller directly.
- ASC ownership follows Unreal best practices: PlayerState for player-controlled pawns, the Pawn itself for AI and standalone pawns.

---

## Actor Class Hierarchy

```
AActor
└── APawn
    ├── AAtlasPawn                     (generic pawn, GAS-ready)
    │   ├── AAtlasCharacter            (humanoid, uses CharacterMovementComponent)
    │   ├── AAtlasVehiclePawn          (vehicle, no CharacterMovement)
    │   └── AAtlasMountPawn            (rideable, can be possessed by mount rider)
└── AController
    ├── AAtlasPlayerController         (player controller, manages input extension)
    └── AAtlasAIController             (AI controller)
AGameModeBase
└── AAtlasGameMode
AGameStateBase
└── AAtlasGameState
APlayerState
└── AAtlasPlayerState                  (owns ASC for player-controlled pawns)
```

---

## `AAtlasPawn` (extend existing)

**Responsibility:** Base for all Atlas pawns. Hosts the component contract that all GAS-ready pawns follow.
**Key changes from current stub:**
- Add `UAtlasPawnExtensionComponent* PawnExtComp` (created in constructor)
- `BeginPlay()` → `PawnExtComp->InitializeAbilitySystem()`
- `PossessedBy(AController*)` → `PawnExtComp->HandleControllerChanged()`
- `UnPossessed()` → `PawnExtComp->HandleControllerChanged()`
- `OnRep_Controller()` → `PawnExtComp->HandleControllerChanged()` (client)
- `GetAbilitySystemComponent()` → delegate to `PawnExtComp`

---

## `AAtlasCharacter` (extend existing)

**Extends:** `AAtlasPawn` (change base from `ACharacter` — use `AAtlasPawn` and include `UCharacterMovementComponent` directly, OR keep `ACharacter` and duplicate extension hooks)

**Recommended approach:** Keep `AAtlasCharacter` extending `ACharacter` to retain `CharacterMovementComponent` and native animation support. Override `GetAbilitySystemComponent()` from `IAbilitySystemInterface` via `PawnExtComp`.

**Additional hooks:**
- `InitializeCharacter()` — calls super, then notifies `PawnExtComp`
- `SetupPlayerInputComponent(UInputComponent*)` → `InputExtComp->BindInputMappingContexts()`

---

## `AAtlasVehiclePawn` *(new)*

**Extends:** `AAtlasPawn`
**Responsibility:** Base for all vehicle pawns. Does not implement physics vehicle movement. Provides extension point contract.
**Key components:**
- `UAtlasPawnExtensionComponent` (from `AAtlasPawn`)
- `UAtlasVehicleExtensionComponent` — handles enter/exit lifecycle
- `UAtlasMovementExtensionComponent` — mode tracking
**Extension points:**
- `OnPassengerEntered(APawn* Passenger, int32 SeatIndex)`
- `OnPassengerExited(APawn* Passenger, int32 SeatIndex)`
- `GetAvailableSeats() const`

---

## `AAtlasMountPawn` *(new)*

**Extends:** `AAtlasPawn`
**Responsibility:** Base for rideable mounts. Supports a single rider via mount lifecycle components.
**Key components:**
- `UAtlasPawnExtensionComponent`
- `UAtlasMountExtensionComponent` — handles rider attachment, input routing during mount
- `UAtlasMovementExtensionComponent`
**Extension points:**
- `OnRiderMounted(APawn* Rider)`
- `OnRiderDismounted(APawn* Rider)`

---

## `AAtlasPlayerController` (extend existing)

**Responsibility:** Player controller. Owns `UAtlasInputExtensionComponent`. Routes input to the possessed pawn's `InputExtComp`. Manages input mode on possession change.
**Key changes:**
- Add `UAtlasInputExtensionComponent* InputExtComp`
- `OnPossess(APawn*)` → set `InputExtComp` target pawn, call `InputExtComp->ApplyInputMappingContexts()`
- `OnUnPossess()` → clear input mapping contexts
- `SetupInputComponent()` → `InputExtComp->BindNativeActions(EnhancedInputComponent)`

---

## `AAtlasAIController` (extend existing)

**Responsibility:** AI controller. Does not use Enhanced Input. Coordinates with the pawn's `PawnExtComp` for ability system initialization (AI-owned ASC).
**Key changes:**
- `OnPossess(APawn*)` → call `PawnExtComp->InitializeAbilitySystemForAI(this)`
- `OnUnPossess()` → call `PawnExtComp->UninitializeAbilitySystem()`

---

## `AAtlasGameMode` (extend existing)

**Recommendation:** Change base to `AGameMode` from `AGameModeBase` to support match state.
**Key additions:**
- Expose `DefaultPawnData` as a `TSoftObjectPtr<UAtlasPawnData>` to allow per-map pawn configuration
- Override `GetDefaultPawnClassForController()` to resolve from `UAtlasPawnData`
- `StartPlay()` → `InitializeGameMode()` → configure default classes from settings/data assets

---

## `AAtlasPlayerState` (extend existing)

**Responsibility:** Owns the `UAtlasAbilitySystemComponent` for player-controlled pawns.
**Key additions:**
- `UAtlasAbilitySystemComponent* AbilitySystemComponent`
- Implements `IAbilitySystemInterface`
- `GetAbilitySystemComponent()` returns the ASC
- `BeginPlay()` → ASC initialization (client replication-safe)

---

## Component Architecture

### `UAtlasPawnExtensionComponent` *(new)*

**Responsibility:** The coordination hub for GAS initialization on a pawn. Holds a reference to `UAtlasPawnData`. Waits for all required conditions (pawn spawned, controller set, ASC ready) before granting abilities.

**Key API:**
```cpp
// Set from pawn constructor or from game mode
void SetPawnData(const UAtlasPawnData* InPawnData);

// Called by pawn's PossessedBy / OnRep_Controller
void HandleControllerChanged();

// Called when ASC is ready (either from PlayerState rep or from pawn itself for AI)
void InitializeAbilitySystem(UAtlasAbilitySystemComponent* ASC, AActor* OwnerActor);
void UninitializeAbilitySystem();

// Query
bool IsAbilitySystemReady() const;
UAtlasPawnData* GetPawnData() const;
UAtlasAbilitySystemComponent* GetAbilitySystemComponent() const;

// Delegates
FSimpleMulticastDelegate OnAbilitySystemInitialized;
FSimpleMulticastDelegate OnAbilitySystemUninitialized;
```

**Initialization flow:**
1. Pawn constructor creates `PawnExtComp`
2. Game mode or feature plugin calls `SetPawnData()`
3. `PossessedBy` fires → `HandleControllerChanged()`
4. For player: PlayerState's ASC is replicated → `InitializeAbilitySystem()` called from PlayerState
5. For AI: `AAtlasAIController::OnPossess()` calls `InitializeAbilitySystem()` with pawn-owned ASC
6. `PawnExtComp` calls `AbilityExtComp->GrantAbilitySet()` from the pawn data

---

### `UAtlasAbilityExtensionComponent` *(new)*

**Responsibility:** Grants ability sets, attribute sets, and startup effects to the pawn when the ASC is initialized.

**Key API:**
```cpp
void GrantAbilitySet(const UAtlasAbilitySet* AbilitySet, UAtlasAbilitySystemComponent* ASC);
void RevokeAbilitySet(const UAtlasAbilitySet* AbilitySet, UAtlasAbilitySystemComponent* ASC);
```

---

### `UAtlasInputExtensionComponent` *(new)*

**Responsibility:** Manages Enhanced Input mapping contexts for the pawn. Binds input actions to abilities via the ASC.

**Key API:**
```cpp
void ApplyInputMappingContexts(APlayerController* PC, const TArray<FAtlasInputConfig>& Configs);
void RemoveInputMappingContexts(APlayerController* PC);
void BindAbilityInputActions(UEnhancedInputComponent* InputComp, UAtlasAbilitySystemComponent* ASC);
```

---

### `UAtlasMovementExtensionComponent` *(new)*

**Responsibility:** Tracks the current movement mode (walking, swimming, flying, vehicle, mounted). Routes movement queries to the appropriate movement component.

**Key API:**
```cpp
EAtlasMovementMode GetCurrentMovementMode() const;
void RequestMovementModeChange(EAtlasMovementMode NewMode);

// Delegates
FAtlasMovementModeChangedDelegate OnMovementModeChanged;
```

See `06_movement.md` for full movement design.

---

### `UAtlasVehicleExtensionComponent` *(new)*

**Responsibility:** Handles the enter/exit lifecycle for vehicle pawns. Manages seat assignment and passenger tracking.

**Key API:**
```cpp
bool TryEnterVehicle(APawn* Passenger, int32 SeatIndex = 0);
void ExitVehicle(APawn* Passenger);
bool IsSeatOccupied(int32 SeatIndex) const;
int32 GetPassengerCount() const;

// Delegates
FAtlasVehiclePassengerDelegate OnPassengerEntered;
FAtlasVehiclePassengerDelegate OnPassengerExited;
```

---

### `UAtlasMountExtensionComponent` *(new)*

**Responsibility:** Handles mount/dismount lifecycle. Manages rider attachment to the mount socket, input routing while mounted.

**Key API:**
```cpp
bool TryMount(APawn* Rider);
void Dismount(APawn* Rider);
bool IsMounted() const;
APawn* GetCurrentRider() const;

// Delegates
FAtlasMountRiderDelegate OnRiderMounted;
FAtlasMountRiderDelegate OnRiderDismounted;
```

---

## GAS Ownership Rules

| Pawn Type | ASC Owner | ASC Lives On |
|---|---|---|
| Player-controlled character/pawn | `AAtlasPlayerState` | Replicated from server, long-lived across possessions |
| AI-controlled character/pawn | The pawn itself | Destroyed when pawn is destroyed |
| Vehicle (player-entered) | Passenger's PlayerState | No new ASC for vehicle; passenger's ASC persists |
| Mount (player-riding) | Rider's PlayerState | No new ASC for mount; rider's ASC persists |

**Rationale for PlayerState-owned ASC:** The player's abilities and attributes must survive pawn destruction and respawn. If the ASC were pawn-owned, abilities would be lost every time the pawn is destroyed. PlayerState-owned ASC is the standard Lyra/Epic approach for multiplayer-ready templates.

---

## Pawn Data Asset — `UAtlasPawnData`

**Module:** `GameActors`
**Extends:** `UPrimaryDataAsset`
**Responsibility:** Per-pawn-type configuration. Defines everything needed to set up a pawn instance without hardcoding in actor classes.

```cpp
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasPawnData : public UPrimaryDataAsset
{
    // Pawn class to spawn for this data
    UPROPERTY(EditDefaultsOnly)
    TSoftClassPtr<APawn> PawnClass;

    // Abilities and effects granted on possession
    UPROPERTY(EditDefaultsOnly)
    TArray<TSoftObjectPtr<UAtlasAbilitySet>> AbilitySets;

    // Input mapping contexts applied to the player controller
    UPROPERTY(EditDefaultsOnly)
    TArray<FAtlasInputConfig> InputConfigs;

    // Camera/spring arm configuration
    UPROPERTY(EditDefaultsOnly)
    TSoftObjectPtr<UAtlasCameraConfig> CameraConfig;
};
```
