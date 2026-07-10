#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AtlasSavableComponent.generated.h"

class AActor;

/*
 * Gives an actor a stable identity for the save/load pipeline.
 *
 * The save collector only captures actors carrying this component; the load
 * system resolves (or respawns) actors by the ActorId GUID. The ID is
 * generated once and persisted with the actor; duplicating an actor in the
 * editor generates a fresh ID so two actors never share one.
 */
UCLASS(ClassGroup = (Atlas), meta = (BlueprintSpawnableComponent))
class GAMECORE_API UAtlasSavableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasSavableComponent();

	//~UActorComponent interface
	// Each override guarantees a valid, unique ActorId at the earliest
	// opportunity for its path (placement, asset load, duplication).
	virtual void OnRegister() override;
	virtual void PostLoad() override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	//~End of UActorComponent interface

	const FGuid& GetActorId() const;
	bool HasValidActorId() const;

	/*
	 * Generates an ID if none exists yet; keeps an existing one.
	 */
	void EnsureActorId();

	/*
	 * Overrides the ID; used by the load system when respawning an actor
	 * recorded in a snapshot.
	 */
	void SetActorId(const FGuid& InActorId);

	static UAtlasSavableComponent* FindOnActor(const AActor* Actor);

private:
	void GenerateActorId();

	// Stable per-actor identity; SaveGame so it round-trips through
	// serialization with the actor.
	UPROPERTY(VisibleInstanceOnly, SaveGame, Category = "Savable")
	FGuid ActorId;
};
