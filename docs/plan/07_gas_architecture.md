# 07 — Gameplay Ability System Architecture

All classes live in `GameActors` unless stated otherwise.

---

## Required Engine Modules

Add to `GameActors.Build.cs` public dependencies:
```csharp
"GameplayAbilities", "GameplayTags", "GameplayTasks"
```

These are already in `GameCore.Build.cs` — they must be added to `GameActors.Build.cs` as well since the actor classes will use GAS types directly.

Also enable in `Game.uproject` plugins section:
```json
{ "Name": "GameplayAbilities", "Enabled": true }
```

---

## `UAtlasAbilitySystemComponent`

**Module:** `GameActors`
**Extends:** `UAbilitySystemComponent`
**Responsibility:** Custom ASC. Provides Atlas-specific hooks and input ability activation without changing core GAS behavior.

**Additions over base ASC:**
```cpp
// Ability input binding — called by UAtlasInputExtensionComponent
void AbilityInputTagPressed(const FGameplayTag& InputTag);
void AbilityInputTagReleased(const FGameplayTag& InputTag);

// Grants an entire ability set in one call
FAtlasAbilitySetHandle GrantAbilitySet(const UAtlasAbilitySet* AbilitySet);
void RevokeAbilitySet(FAtlasAbilitySetHandle Handle);

// Delegate — fired after all startup abilities are granted
FSimpleMulticastDelegate OnAbilitiesGranted;
```

**Replication:** Uses UE GAS default replication — mixed replication model (full on owning client, minimal on simulated proxies). No custom replication needed in the Atlas layer.

---

## `UAtlasBaseAttributeSet`

**Module:** `GameActors`
**Extends:** `UAttributeSet`
**Responsibility:** Minimal shared attribute set. Does not contain game-specific attributes (health, stamina, etc.). Provides the `PreAttributeChange` / `PostGameplayEffectExecute` override pattern for subclasses.

**Contents:**
```cpp
// Example framework-level attribute — not game-specific
UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Level)
FGameplayAttributeData Level;

// Clamp helper — called in PreAttributeChange
static float ClampAttributeOnChange(float NewValue, float Min, float Max);

// Required overrides
virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
```

Game projects subclass `UAtlasBaseAttributeSet` to add game-specific attributes (Health, Mana, Stamina, etc.).

---

## `UAtlasBaseGameplayAbility`

**Module:** `GameActors`
**Extends:** `UGameplayAbility`
**Responsibility:** Base ability class. Adds Atlas-specific helpers and enforces input tag binding convention.

**Additions:**
```cpp
// The input tag that activates this ability (matches FAtlasAbilityInputConfig::InputTag)
UPROPERTY(EditDefaultsOnly, Category = "Atlas|Input")
FGameplayTag InputTag;

// Instancing policy default override — prefer InstancedPerActor
// (set in the constructor: InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor)

// Helper: get the owning Atlas pawn
AAtlasPawn* GetAtlasPawn() const;

// Helper: get the owning Atlas player controller
AAtlasPlayerController* GetAtlasPlayerController() const;

// Helper: get the owning ASC cast to Atlas type
UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponent() const;
```

Game projects subclass `UAtlasBaseGameplayAbility` for all concrete abilities.

---

## `UAtlasAbilitySet`

**Module:** `GameActors`
**Extends:** `UPrimaryDataAsset`
**Responsibility:** Data-driven bundle of abilities, effects, and attribute sets to grant to a pawn. The pawn data references ability sets; the extension component grants them.

```cpp
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasAbilitySet : public UPrimaryDataAsset
{
    // Abilities to grant
    UPROPERTY(EditDefaultsOnly)
    TArray<FAtlasGrantedAbility> GrantedAbilities;

    // Attribute sets to add
    UPROPERTY(EditDefaultsOnly)
    TArray<TSoftClassPtr<UAtlasBaseAttributeSet>> GrantedAttributeSets;

    // Startup effects to apply (passive effects, base value init)
    UPROPERTY(EditDefaultsOnly)
    TArray<FAtlasGrantedGameplayEffect> GrantedEffects;
};

USTRUCT(BlueprintType)
struct FAtlasGrantedAbility
{
    TSoftClassPtr<UAtlasBaseGameplayAbility> AbilityClass;
    int32 AbilityLevel = 1;
    FGameplayTag InputTag;  // which input activates this ability
};

USTRUCT(BlueprintType)
struct FAtlasGrantedGameplayEffect
{
    TSoftClassPtr<UGameplayEffect> EffectClass;
    float Level = 1.0f;
};

struct FAtlasAbilitySetHandle
{
    TArray<FGameplayAbilitySpecHandle> AbilityHandles;
    TArray<TSubclassOf<UAttributeSet>> AttributeSets;
    TArray<FActiveGameplayEffectHandle> EffectHandles;
    bool IsValid() const;
};
```

---

## Input-to-Ability Binding Strategy

**Pattern:** Tag-based activation, not index-based.

Each granted ability has an `InputTag` (e.g., `Atlas.Input.Ability.Primary`). The player controller's Enhanced Input component binds input actions to these tags.

```
InputAction (IA_AbilityPrimary)
    → EnhancedInput binding in UAtlasInputExtensionComponent
        → on Triggered: ASC->AbilityInputTagPressed(Atlas.Input.Ability.Primary)
        → on Completed: ASC->AbilityInputTagReleased(Atlas.Input.Ability.Primary)

UAtlasAbilitySystemComponent receives the tag
    → finds all granted abilities with matching InputTag
    → tries to activate
```

This decouples input actions from specific ability classes. Abilities declare what input they respond to. Input configs declare what actions exist. Pawn data wires them together.

**`FAtlasInputConfig`** (in `GameActors`):
```cpp
USTRUCT(BlueprintType)
struct FAtlasInputConfig
{
    // The input mapping context to apply
    TSoftObjectPtr<UInputMappingContext> InputMappingContext;
    int32 Priority = 0;

    // Maps input actions to gameplay tags for ability activation
    TArray<FAtlasAbilityInputBinding> AbilityBindings;
};

USTRUCT(BlueprintType)
struct FAtlasAbilityInputBinding
{
    TSoftObjectPtr<UInputAction> InputAction;
    FGameplayTag InputTag;
};
```

---

## Initialization Order

### Player-controlled pawn (full sequence):

```
1. AAtlasGameMode spawns pawn class from UAtlasPawnData
2. AAtlasPlayerController::OnPossess(Pawn)
3. Pawn::PossessedBy(PC) → PawnExtComp::HandleControllerChanged()
4. PlayerState replicates to client (or is already available on server)
5. PlayerState::BeginPlay() → ASC initialized (server)
6. PawnExtComp::InitializeAbilitySystem(PlayerState->ASC, PlayerState)
7. AbilityExtComp::GrantAbilitySet(PawnData->AbilitySets)
8. InputExtComp::ApplyInputMappingContexts(PC, PawnData->InputConfigs)
9. InputExtComp::BindAbilityInputActions(EnhancedInputComp, ASC)
10. PawnExtComp::OnAbilitySystemInitialized fires
```

On client:
- Steps 1–3 happen (pawn spawns)
- Step 4: `OnRep_PlayerState()` fires → triggers step 6 on client too
- Steps 7–9 happen on client through `PawnExtComp::OnAbilitySystemInitialized`

### AI-controlled pawn:

```
1. AAtlasAIController::OnPossess(Pawn)
2. Pawn creates its own ASC (pawn-owned, not PlayerState-owned)
3. PawnExtComp::InitializeAbilitySystem(Pawn->ASC, Pawn)
4. AbilityExtComp::GrantAbilitySet(PawnData->AbilitySets)
5. No input binding needed
6. PawnExtComp::OnAbilitySystemInitialized fires
```

### Vehicle-possessed player:

When a player enters a vehicle:
- Player's own ASC **persists** on their PlayerState
- No new ASC is created for the vehicle
- The vehicle's abilities (if any) are granted to the player's ASC from the vehicle's own `UAtlasAbilitySet`
- On exit, vehicle-granted abilities are revoked

---

## Gameplay Effect Strategy

| Effect Type | When to use |
|---|---|
| Instant | Attribute modifications that happen once (damage, healing, XP) |
| Duration | Temporary buffs/debuffs with a time limit |
| Infinite | Passive effects while a condition holds (e.g., "on mount" speed boost) — manually removed |
| Startup (from ability set) | Attribute base value initialization (set character's base Health Max) |

Game projects define all concrete effects. The Atlas layer provides:
- `UAtlasBaseGameplayEffect` — thin subclass with Atlas-specific default tags
- Tag conventions: `Atlas.Effect.Duration.*`, `Atlas.Effect.Instant.*`, `Atlas.Effect.Passive.*`

---

## Gameplay Tag Conventions for GAS

```
Atlas.Ability.Category.*      — ability category tags (for filtering/cancellation)
Atlas.Ability.Block.*         — tags that block ability activation
Atlas.Ability.Cancel.*        — tags that cancel abilities
Atlas.Input.Ability.*         — input binding tags (used for activation)
Atlas.Effect.*                — effect classification tags
Atlas.State.*                 — gameplay state tags (dead, stunned, sprinting)
Atlas.Attribute.*             — attribute identity tags
```

---

## Replication Considerations

- ASC on PlayerState: `APlayerState` is replicated. ASC must call `InitAbilityActorInfo(OwnerActor, AvatarActor)` — `OwnerActor = PlayerState`, `AvatarActor = Pawn`.
- When pawn is destroyed and respawned: call `InitAbilityActorInfo` again with the new pawn as `AvatarActor`. The PlayerState and its ASC survive.
- Minimum replication mode: `EGameplayEffectReplicationMode::Mixed` — full replication to owning client, tag/count replication to simulated proxies.
- Atlas sets this default in `UAtlasAbilitySystemComponent`'s constructor.

---

## Game Feature Plugin Integration

Game Feature plugins extend GAS by:
1. Providing additional `UAtlasAbilitySet` data assets
2. Using `UGameFeatureAction_AddAbilities` (Lyra-style custom action) to grant ability sets to specific pawn types when the feature activates
3. Using `UGameFeatureAction_AddInputConfig` to register additional input mapping contexts
4. Never hardcoding pawn classes — always targeting by `FGameplayTag` or pawn data type

The base project does **not** hard-depend on any specific ability sets. All abilities come from feature plugins or game-project pawn data assets.
