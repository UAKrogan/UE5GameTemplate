# Gameplay Ability System

How the Atlas template sets up GAS (`GameActors`). For GAS fundamentals see Epic's documentation; this guide covers only the Atlas layer.

## Building blocks

| Type | Role |
|---|---|
| `UAtlasAbilitySystemComponent` | Custom ASC: Mixed replication, tag-based input activation, ability-set granting |
| `UAtlasBaseAttributeSet` | Attribute set base with the clamp/rep-notify pattern; games subclass to add Health etc. |
| `UAtlasBaseGameplayAbility` | Ability base: `InstancedPerActor`, avatar/controller/ASC helpers, `InputTag` |
| `UAtlasAbilitySet` | Data asset bundling abilities + startup effects + attribute sets |
| `UAtlasPawnData` | Data asset: pawn class + ability sets + input configs |
| `UAtlasPawnExtensionComponent` | Coordination hub: initializes the ASC on possession, grants pawn data sets |
| `UAtlasAbilityExtensionComponent` | Creates the pawn-owned ASC for AI pawns |

## ASC ownership

| Pawn type | ASC owner | Why |
|---|---|---|
| Player-controlled | `AAtlasPlayerState` | Abilities/attributes survive pawn death and respawn (Lyra pattern) |
| AI-controlled | The pawn itself | ASC lives and dies with the pawn |

Replication mode is `Mixed` (full to owning client, tags/cues to simulated proxies), set in the ASC constructor. `AAtlasPlayerState` raises its net update frequency to 100 Hz for attribute responsiveness.

## Initialization order

**Player-controlled pawn:**
1. Game mode spawns the pawn (class resolved from `DefaultPawnData` when set).
2. `PossessedBy` → `PawnExtComp->HandleControllerChanged()`.
3. PlayerState (and its ASC) is found → `InitializeAbilitySystem(PS->ASC, PS)`: `InitAbilityActorInfo(PlayerState, Pawn)`.
4. On the authority, pawn data ability sets are granted.
5. On clients, `OnRep_PlayerState` re-runs `HandleControllerChanged()` — the PlayerState can replicate after the controller, so this is the client-side init trigger.
6. `OnAbilitySystemInitialized` fires; input binding happens in `SetupPlayerInputComponent`.

**AI pawn:** `AAtlasAIController::OnPossess` → `AbilityExtComp->GetOrCreateAbilitySystemComponent()` → `PawnExtComp->InitializeAbilitySystem(ASC, Pawn)`. `OnUnPossess` uninitializes and revokes.

## Authoring an ability set

1. Subclass `UAtlasBaseGameplayAbility` (C++ or Blueprint).
2. Create a `UAtlasAbilitySet` asset:
   - `GrantedAbilities`: ability class + level + `InputTag` (e.g. `Atlas.Input.Ability.Jump`)
   - `GrantedAttributeSets`: attribute set classes to spawn on the owner
   - `GrantedEffects`: startup effects (attribute base value init, passives)
3. Reference the set from a `UAtlasPawnData` (granted on possession) or grant manually:

```cpp
FAtlasAbilitySetHandle Handle = ASC->GrantAbilitySet(AbilitySet);   // authority only
ASC->RevokeAbilitySet(Handle);                                      // removes everything as a unit
```

Order inside a grant: attribute sets → effects → abilities, so startup effects can initialize attribute values.

## Input-to-ability binding

Tag-based, never index-based:

```
IA_Jump ─(UAtlasInputConfigData binding)─► Atlas.Input.Ability.Jump
    Started   → ASC->AbilityInputTagPressed(Tag)
    Completed → ASC->AbilityInputTagReleased(Tag)
ASC finds granted specs whose dynamic source tags contain the tag → TryActivateAbility
```

The `InputTag` from `FAtlasGrantedAbility` is added to the spec's dynamic source tags at grant time. Input actions never reference ability classes; abilities declare what input they respond to; pawn data wires them together. See [actors.md](actors.md) for the input extension component flow.

## Subclassing the attribute set

```cpp
UCLASS()
class UMyAttributeSet : public UAtlasBaseAttributeSet
{
    GENERATED_BODY()
public:
    ATLAS_ATTRIBUTE_ACCESSORS(UMyAttributeSet, Health);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
    FGameplayAttributeData Health;
    // clamp in PreAttributeChange, react in PostGameplayEffectExecute,
    // DOREPLIFETIME_CONDITION_NOTIFY in GetLifetimeReplicatedProps
};
```

## Effect strategy

| Effect type | Use for |
|---|---|
| Instant | One-shot attribute changes (damage, healing) |
| Duration | Timed buffs/debuffs |
| Infinite | Passives while a condition holds — removed manually |
| Startup (from ability set) | Attribute base value initialization |

## Debugging

- `showdebug abilitysystem` in PIE (backtick console) — verify ASC owner/avatar, granted abilities, attributes.
- Verify grants are authority-side; `GrantAbilitySet` logs a warning when called without authority.
