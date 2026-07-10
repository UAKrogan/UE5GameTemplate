#pragma once

#include "Components/ActorComponent.h"
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

private:
	void HandleAbilityInputPressed(FGameplayTag InputTag);
	void HandleAbilityInputReleased(FGameplayTag InputTag);

	UPROPERTY(Transient)
	TArray<TObjectPtr<const UAtlasInputConfigData>> AppliedConfigs;

	TWeakObjectPtr<APawn> BoundPawn;
};
