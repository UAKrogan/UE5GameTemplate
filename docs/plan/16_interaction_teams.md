# 16 — Interaction & Teams Foundation

All classes live in `GameActors` unless stated otherwise. Two independent genre-agnostic services: a world-interaction contract (RPG core loop, usable by RTS capture points) and faction/attitude plumbing (both genres).

---

## Interaction

### Contract — `IAtlasInteractable` (UINTERFACE)

Implemented by actors or components that can be used/picked up/talked to.

```cpp
class GAMEACTORS_API IAtlasInteractable
{
public:
    // Whether this target can currently be interacted with by the instigator.
    virtual bool CanInteract(APawn* Instigator) const = 0;

    // Display data for prompts ("Open", key glyph resolved by UI via the
    // glyph subsystem and the interaction input action).
    virtual FText GetInteractionVerb() const = 0;

    // Perform the interaction. Authority-side; cosmetic reactions via
    // multicast/game code.
    virtual void Interact(APawn* Instigator) = 0;

    // Focus highlight hooks (outline, UI marker). Cosmetic, owning client.
    virtual void OnFocusGained(APawn* Instigator) {}
    virtual void OnFocusLost(APawn* Instigator) {}
};
```

Plus `UAtlasInteractableComponent`: a drop-in `UActorComponent` implementing the interface with Blueprint-assignable events and an `InteractionVerb` property, so designers never need C++ for simple interactables.

### Detector — `UAtlasInteractionComponent` (on the pawn)

- Finds the current best interactable each scan: configurable detection — `LineTrace` (FP/TP look-at), `Overlap` (proximity sphere), or `CursorTrace` (RTS/point-and-click: deprojects the mouse via the player controller).
- Scan cadence: timer-driven (default 10 Hz), not per-tick.
- Tracks `CurrentFocus` (weak); fires `OnFocusChanged(OldInteractable, NewInteractable)` — game HUD widgets bind this to show/hide prompts (via existing HUD element slots; no source `GameUI` changes).
- `TryInteract()`: validates `CanInteract` and calls `Interact` on the authority (server RPC path documented; single-player works directly).
- Input binding: framework ability `UAtlasInteractAbility` (granted via ability set, bound to `Atlas.Input.Ability.Interact`) that calls `TryInteract()` — consistent with the tag-based input pipeline, no new input plumbing.

### New native tags

```
Atlas.Input.Ability.Interact
Atlas.Interaction.Blocked      (state tag an ASC can apply to disable interaction)
```

---

## Teams / factions

Wire UE's `IGenericTeamAgentInterface` (already used by `AAIController` perception) through the Atlas actors, with team definitions as data.

### Types

```cpp
// GameActors/Public/Teams/AtlasTeamTypes.h
UENUM(BlueprintType)
enum class EAtlasTeamAttitude : uint8 { Friendly, Neutral, Hostile };

// One row per team pair override; default attitude configurable.
UCLASS(BlueprintType) // UDataAsset
class UAtlasTeamConfig : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly) TMap<uint8, EAtlasTeamAttitude> AttitudeOverrides; // packed pair key helper
    UPROPERTY(EditDefaultsOnly) EAtlasTeamAttitude DefaultAttitude = EAtlasTeamAttitude::Hostile;
};
```

### Integration points

- `UAtlasTeamComponent` (on any actor): holds `FGenericTeamId TeamId` (replicated) + `OnTeamChanged` delegate; single source of truth for an actor's team.
- `AAtlasPlayerState` and `AAtlasAIController` implement `IGenericTeamAgentInterface`, resolving through the pawn's/state's team component; `AAtlasAIController::GetTeamAttitudeTowards()` consults the team config (soft ref in `UAtlasDeveloperSettings` — stored as soft path in GameCore settings, loaded in GameActors, consistent with existing cross-module bridging).
- Static helper `UAtlasTeamStatics::GetAttitude(const AActor* A, const AActor* B)` for gameplay/GAS queries (targeting filters, RTS command validation).
- Optional `Atlas.Team.*` tags kept out for now — `FGenericTeamId` is the runtime identity; tags can map onto it later if needed.

### Genre usage

- **RTS**: selection filtering (own units only), order validation (attack requires Hostile), AI perception affiliation masks work out of the box.
- **RPG**: hostiles vs friendlies for abilities/AI aggro; companions share the player team.

---

## Module/dependency impact

- All in `GameActors`; `AIModule` (already a dep) provides `IGenericTeamAgentInterface`.
- `UAtlasDeveloperSettings` (GameCore) gains a `TeamConfig` soft path.
- Save: `UAtlasTeamComponent::TeamId` marked `SaveGame`; interactables persist state via the existing `IAtlasSavable` route when needed.

## How to test

1. Place a `UAtlasInteractableComponent` actor; walk up in TP → `OnFocusChanged` fires, prompt event carries the verb; press Interact → Blueprint event fires once, authority-side.
2. Switch detection to CursorTrace on a top-down pawn → hovering the actor focuses it.
3. Two pawns with team components 1 and 2, config says Hostile → `GetAttitude` returns Hostile; AI perception on an `AAtlasAIController` reports the enemy; setting both to team 1 flips to Friendly.
