#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Systems/Savable/AtlasSavableTypes.h"
#include "AtlasSavable.generated.h"

UINTERFACE()
class GAMECORE_API UAtlasSavable : public UInterface
{
	GENERATED_BODY()
};

/*
 * Implement on Actors or Components that want to participate in Atlas save/load.
 *
 * The actor must also carry a UAtlasSavableComponent (stable FGuid identity)
 * so the collector discovers it and the load system can resolve it later.
 * Both hooks run on the game thread.
 */
class GAMECORE_API IAtlasSavable
{
	GENERATED_BODY()

public:
	/*
	 * Writes this object's state into the save context. Called by the save
	 * collector while building the world snapshot, before any async work.
	 */
	virtual void CaptureState(FAtlasSaveContext& Context) = 0;

	/*
	 * Restores state previously written by CaptureState. Called by the load
	 * system after the owning actor has been resolved or spawned.
	 */
	virtual void RestoreState(const FAtlasLoadContext& Context) = 0;
};
