#pragma once

#include "SaveGame/AtlasSaveGameTypes.h"
#include "Systems/Load/IAtlasLoadSystem.h"
#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

class AActor;
class UActorComponent;
class UAtlasGameInstanceSubsystem;
class UAtlasSavableComponent;
class UObject;
class UWorld;

/*
 * Concrete implementation of Load system.
 *
 * Handles:
 * - Reading binary save data asynchronously
 * - Deserializing world snapshots asynchronously
 * - Resolving/spawning actors on the game thread
 * - Restoring actor and component state on the game thread
 */
class GAMECORE_API FAtlasLoadSystem : public IAtlasLoadSystem, public TSharedFromThis<FAtlasLoadSystem>
{
public:
	virtual void Initialize(UAtlasGameInstanceSubsystem* Subsystem) override;
	virtual void Shutdown() override;

	virtual FName GetSystemName() const override
	{
		return "LoadSystem";
	}

	virtual bool SupportsInterface(FName InterfaceName) const override
	{
		return InterfaceName == IAtlasLoadSystem::InterfaceName();
	}

	virtual void LoadGame() override;
	virtual bool RequestLoad(const FString& SlotName) override;
	virtual bool LoadGameFromSlot(const FString& SlotName, int32 UserIndex = 0) override;
	virtual FString GetDefaultSlotName() const override;
	virtual bool DoesSaveExist(const FString& SlotName, int32 UserIndex = 0) const override;
	virtual void ClearLoadedSnapshot() override;
	virtual bool GetLoadedMetadata(FAtlasSaveGameMetadata& OutMetadata) const override;
	virtual bool GetLoadedSnapshot(FAtlasSaveGameSnapshot& OutSnapshot) const override;
	virtual bool TryGetIntValue(FName Key, int32& OutValue) const override;
	virtual bool TryGetFloatValue(FName Key, float& OutValue) const override;
	virtual bool TryGetStringValue(FName Key, FString& OutValue) const override;
	virtual bool TryGetVectorValue(FName Key, FVector& OutValue) const override;
	virtual bool TryGetRotatorValue(FName Key, FRotator& OutValue) const override;
	virtual bool TryGetTransformValue(FName Key, FTransform& OutValue) const override;

private:
	UWorld* ResolveWorld() const;
	AActor* FindActorById(UWorld* World, const FGuid& ActorId) const;
	AActor* SpawnActorFromSnapshot(UWorld* World, const FAtlasActorSnapshot& ActorSnapshot) const;
	UAtlasSavableComponent* EnsureSavableComponent(AActor* Actor, const FGuid& ActorId) const;

	void HandleSnapshotLoaded(const FString& SlotName, bool bSuccess, FAtlasWorldSnapshot&& WorldSnapshot);
	void ApplyWorldSnapshot(const FString& SlotName, const FAtlasWorldSnapshot& WorldSnapshot);
	void ResolveActorsForSnapshot(UWorld* World, const FAtlasWorldSnapshot& WorldSnapshot, TMap<FGuid, TWeakObjectPtr<AActor>>& OutActorsById);
	void RestoreSnapshotState(const FAtlasWorldSnapshot& WorldSnapshot, const TMap<FGuid, TWeakObjectPtr<AActor>>& ActorsById);
	void RestoreActorState(AActor* Actor, const FAtlasActorSnapshot& ActorSnapshot);
	void RestoreComponentState(AActor* Actor, const FAtlasActorSnapshot& ActorSnapshot);
	bool RestoreSavableObject(UObject* Object, const FAtlasDataChunk& DataChunk);

	static FName BuildActorChunkName();
	static FName BuildComponentChunkName(const UActorComponent* Component, int32 ComponentIndex);

	/*
	 * Reference to owning subsystem. Raw pointer is safe because the subsystem
	 * owns this system and calls Shutdown() (which clears the pointer) before
	 * it is destroyed. Never cache or use it after Shutdown().
	 */
	UAtlasGameInstanceSubsystem* OwningSubsystem = nullptr;

	FAtlasSaveGameSnapshot LoadedSnapshot;
	FAtlasWorldSnapshot LoadedWorldSnapshot;

	// Only accessed on the game thread. Async callbacks must be dispatched via
	// AsyncTask(ENamedThreads::GameThread, ...) before reading or writing these flags.
	bool bLoadInProgress = false;
	bool bShuttingDown = false;
};
