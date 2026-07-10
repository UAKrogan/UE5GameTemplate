# Actors and Extension Components

The `GameActors` hierarchy and the component contract that keeps base classes thin.

## Hierarchy

```
APawn
└── AAtlasPawn                 (generic pawn, GAS-ready)
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

Tracks the high-level movement mode (`Ground`, `Swimming`, `Flying`, `Vehicle`, `Mounted`) so gameplay/animation can react to transitions without knowing the driving movement component. `RequestMovementModeChange()` broadcasts `OnMovementModeChanged`. Vehicle/mount pawn bases and their extension components are future work (see `docs/plan/06_movement.md`).

## Save participation

- Add `UAtlasSavableComponent` to give the actor a stable `FGuid ActorId`.
- Implement `IAtlasSavable` (`CaptureState` / `RestoreState`) on the actor or its components.
- `UAtlasGASSaveAdapter` — drop-in component capturing/restoring ASC attributes and active effects.
- `UAtlasInventorySaveAdapter` — stub for future inventory systems.

The save collector picks up any actor with a savable component automatically; the load system resolves actors by GUID, spawning missing ones from their recorded class.
