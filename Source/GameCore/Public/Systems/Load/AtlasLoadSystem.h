#pragma once

#include "SaveGame/AtlasSaveGameTypes.h"
#include "Systems/Load/IAtlasLoadSystem.h"

class UAtlasGameInstanceSubsystem;

/*
 * Concrete implementation of Load system.
 *
 * Handles:
 * - Restoring saved data
 * - Reconstructing runtime state
 */
class GAMECORE_API FAtlasLoadSystem : public IAtlasLoadSystem
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
	/*
	 * Reference to owning subsystem.
	 */
	UAtlasGameInstanceSubsystem* OwningSubsystem = nullptr;

	FAtlasSaveGameSnapshot LoadedSnapshot;
};
