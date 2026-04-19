#pragma once

#include "SaveGame/AtlasSaveGameTypes.h"
#include "Systems/Save/IAtlasSaveSystem.h"
#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

class UAtlasGameInstanceSubsystem;
class UWorld;

/*
 * Concrete implementation of Save system.
 *
 * Handles:
 * - Collecting savable data on the game thread
 * - Serializing snapshots asynchronously
 * - Writing binary save data asynchronously
 */
class GAMECORE_API FAtlasSaveSystem : public IAtlasSaveSystem, public TSharedFromThis<FAtlasSaveSystem>
{
public:
	virtual void Initialize(UAtlasGameInstanceSubsystem* Subsystem) override;
	virtual void Shutdown() override;

	virtual FName GetSystemName() const override
	{
		return "SaveSystem";
	}

	virtual bool SupportsInterface(FName InterfaceName) const override
	{
		return InterfaceName == IAtlasSaveSystem::InterfaceName();
	}

	virtual void SaveGame() override;
	virtual bool RequestSave(const FString& SlotName) override;
	virtual bool RequestAutosave() override;
	virtual bool SaveGameToSlot(const FString& SlotName, int32 UserIndex = 0) override;
	virtual FString GetDefaultSlotName() const override;
	virtual bool DoesSaveExist(const FString& SlotName, int32 UserIndex = 0) const override;
	virtual void ClearPendingSnapshot() override;
	virtual void SetIntValue(FName Key, int32 Value) override;
	virtual void SetFloatValue(FName Key, float Value) override;
	virtual void SetStringValue(FName Key, const FString& Value) override;
	virtual void SetVectorValue(FName Key, const FVector& Value) override;
	virtual void SetRotatorValue(FName Key, const FRotator& Value) override;
	virtual void SetTransformValue(FName Key, const FTransform& Value) override;
	virtual bool RemoveValue(FName Key) override;
	virtual bool GetPendingSnapshot(FAtlasSaveGameSnapshot& OutSnapshot) const override;

private:
	struct FAtlasQueuedSaveRequest
	{
		FString SlotName;
		bool bAutosave = false;
	};

	FAtlasSaveGameSnapshot BuildSnapshotForSave(const FString& SlotName, int32 UserIndex) const;
	FString ResolveCurrentMapName() const;
	FString GetAutosaveSlotName() const;
	UWorld* ResolveWorld() const;

	bool EnqueueSaveRequest(const FString& SlotName, bool bAutosave);
	void ProcessNextSaveRequest();
	void StartSaveRequest(const FAtlasQueuedSaveRequest& Request);
	void HandleSaveCompleted(const FString& SlotName, bool bAutosave, bool bSuccess, int32 ActorCount, int32 ByteCount);

	/*
	 * Reference to owning subsystem.
	 */
	UAtlasGameInstanceSubsystem* OwningSubsystem = nullptr;

	FAtlasSaveGameSnapshot PendingSnapshot;
	TArray<FAtlasQueuedSaveRequest> PendingSaveRequests;

	bool bSaveInProgress = false;
	bool bShuttingDown = false;
};
