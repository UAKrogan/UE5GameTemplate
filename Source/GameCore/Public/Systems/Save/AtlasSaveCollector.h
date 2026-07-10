#pragma once

#include "CoreMinimal.h"
#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

class AActor;
class UObject;
class UWorld;
class UActorComponent;

/*
 * Builds an in-memory world snapshot from actors that explicitly opt in with
 * UAtlasSavableComponent.
 *
 * This collector does not write to disk and must only run on the game thread.
 */
class GAMECORE_API FAtlasSaveCollector
{
public:
	/*
	 * Walks the world's actors and captures every IAtlasSavable actor and
	 * component into a pure-data snapshot. Each actor gets one chunk for its
	 * own state plus one per savable component.
	 */
	static FAtlasWorldSnapshot CollectWorld(UWorld* World);

private:
	static bool ShouldCollectActor(const AActor* Actor);
	static bool CaptureActor(AActor* Actor, FAtlasActorSnapshot& OutActorSnapshot);
	static bool CaptureSavableObject(UObject* Object, FName ChunkName, FAtlasActorSnapshot& OutActorSnapshot);
	static FName BuildActorChunkName();
	static FName BuildComponentChunkName(const UActorComponent* Component, int32 ComponentIndex);
};
