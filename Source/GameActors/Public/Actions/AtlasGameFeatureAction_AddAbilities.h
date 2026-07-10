#pragma once

#include "Data/AtlasAbilitySet.h"
#include "GameFeatureAction.h"
#include "AtlasGameFeatureAction_AddAbilities.generated.h"

class APawn;
struct FComponentRequestHandle;

/*
 * Grants ability sets to pawns of a given class while the feature is active.
 *
 * Uses the Modular Gameplay extension handler, so pawns spawned after
 * activation are covered too. Grants happen when the pawn's ability system
 * is ready (GameActorReady may fire before possession, in which case the
 * grant is deferred to OnAbilitySystemInitialized).
 */
UCLASS(meta = (DisplayName = "Atlas: Add Abilities"))
class GAMEACTORS_API UAtlasGameFeatureAction_AddAbilities : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureActivating() override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TSoftClassPtr<APawn> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSoftObjectPtr<UAtlasAbilitySet>> AbilitySets;

private:
	void HandleActorExtension(AActor* Actor, FName EventName);
	void TryGrantForActor(AActor* Actor);
	void RevokeForActor(AActor* Actor);
	void Reset();

	TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
	TMap<TWeakObjectPtr<AActor>, TArray<FAtlasAbilitySetHandle>> GrantedHandles;
	// Actors waiting for their ability system to initialize.
	TSet<TWeakObjectPtr<AActor>> PendingActors;
};
