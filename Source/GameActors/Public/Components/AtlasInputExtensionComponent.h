#pragma once

#include "Components/ActorComponent.h"
#include "Components/AtlasMovementExtensionComponent.h"
#include "GameplayTagContainer.h"
#include "AtlasInputExtensionComponent.generated.h"

class APawn;
class UAtlasInputConfigData;
class UEnhancedInputComponent;
class UInputComponent;

/*
 * Owns Enhanced Input wiring for a player controller.
 *
 * On pawn restart it applies the pawn data's input mapping contexts to the
 * local player and binds ability input actions to gameplay tags on the
 * pawn's Enhanced Input component. Tag presses route to the pawn's ASC via
 * AbilityInputTagPressed/Released — input actions never reference ability
 * classes directly.
 *
 * Movement mode integration: while the pawn is InVehicle/Mounted, the pawn
 * data's default mapping contexts are suppressed (mode-specific contexts —
 * drive/ride controls — are applied by the vehicle/mount extension via
 * ApplyConfig). Suppression toggles mapping contexts only, so ability
 * bindings on the pawn's input component are never duplicated.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEACTORS_API UAtlasInputExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasInputExtensionComponent();

	/*
	 * Called from the pawn's SetupPlayerInputComponent (which runs on the
	 * owning client, unlike OnPossess). Applies mapping contexts and binds
	 * ability actions. Bindings live on the pawn's input component and die
	 * with it; mapping contexts are removed in Cleanup().
	 */
	void InitializeForPawn(APawn* InPawn, UInputComponent* PawnInputComponent);

	/*
	 * Removes applied mapping contexts. Called from OnUnPossess.
	 */
	void Cleanup();

	/*
	 * Applies a single config at runtime (mapping context + ability bindings
	 * on the current pawn's input component). Used by Game Feature actions.
	 */
	void ApplyConfig(const UAtlasInputConfigData* Config);

	/*
	 * Removes a runtime-applied config's mapping context. Ability bindings
	 * live on the pawn's input component and are torn down with it on the
	 * next possession change.
	 */
	void RemoveConfig(const UAtlasInputConfigData* Config);

private:
	void HandleAbilityInputPressed(FGameplayTag InputTag);
	void HandleAbilityInputReleased(FGameplayTag InputTag);

	/*
	 * Reacts to the bound pawn's movement mode: suppresses the default
	 * mapping contexts while attached (vehicle/mount), resumes them after.
	 */
	UFUNCTION()
	void HandleMovementModeChanged(EAtlasMovementMode OldMode, EAtlasMovementMode NewMode);

	/*
	 * Adds/removes only the mapping contexts of the pawn data's default
	 * configs — bindings and AppliedConfigs bookkeeping stay untouched.
	 */
	void SetDefaultContextsSuppressed(bool bSuppressed);

	UPROPERTY(Transient)
	TArray<TObjectPtr<const UAtlasInputConfigData>> AppliedConfigs;

	// Subset of AppliedConfigs that came from the pawn data (as opposed to
	// runtime configs from features/vehicles); these get suppressed while
	// the pawn is in an attached movement mode.
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UAtlasInputConfigData>> DefaultConfigs;

	TWeakObjectPtr<APawn> BoundPawn;
	TWeakObjectPtr<UAtlasMovementExtensionComponent> BoundMovementExt;
	bool bDefaultContextsSuppressed = false;
};
