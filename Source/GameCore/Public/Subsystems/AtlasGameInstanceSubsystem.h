#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasGameInstanceSubsystem.generated.h"

class IAtlasSystem;

/*
 * UAtlasGameInstanceSubsystem
 *
 * Central runtime orchestrator for Atlas.
 *
 * Responsibilities:
 * - Own all systems
 * - Register and initialize systems
 * - Shutdown systems
 * - Provide access to systems (service locator pattern)
 *
 * Lifecycle:
 *   Initialize()
 *     -> RegisterSystems()
 *     -> InitializeSystems()
 *
 *   Deinitialize()
 *     -> ShutdownSystems()
 *
 * Notes:
 * - Systems are registered externally (GameSystems module)
 * - Prevents circular dependency between modules
 */
UCLASS()
class GAMECORE_API UAtlasGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*
	 * Called by Unreal during GameInstance initialization.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/*
	 * Called during shutdown.
	 */
	virtual void Deinitialize() override;

	/*
	 * Registers a system instance.
	 *
	 * Called by external modules (e.g., AtlasSystemsRegistry).
	 *
	 * @param System - System instance to register
	 */
	void RegisterSystem(TSharedPtr<IAtlasSystem> System);

	/*
	 * Retrieve a system by interface type.
	 *
	 * Usage example:
	 *
	 *   UAtlasGameInstanceSubsystem* Subsystem =
	 *       GetGameInstance()->GetSubsystem<UAtlasGameInstanceSubsystem>();
	 *
	 *   TSharedPtr<IAtlasSaveSystem> SaveSystem =
	 *       Subsystem->GetSystem<IAtlasSaveSystem>();
	 *
	 *   if (SaveSystem.IsValid())
	 *   {
	 *       SaveSystem->SaveGame();
	 *   }
	 *
	 * Notes:
	 * - Returns nullptr if system not found
	 * - Caller must validate pointer
	 */
	template <typename T>
	TSharedPtr<T> GetSystem() const
	{
		for (const auto& Pair : Systems)
		{
			TSharedPtr<T> Casted = StaticCastSharedPtr<T>(Pair.Value);
			if (Casted.IsValid())
			{
				return Casted;
			}
		}

		return nullptr;
	}

private:
	/*
	 * Delegates system registration to GameSystems module.
	 */
	void RegisterSystems();

	/*
	 * Initializes all registered systems.
	 */
	void InitializeSystems();

	/*
	 * Shuts down all systems.
	 */
	void ShutdownSystems();

	/*
	 * Storage for all runtime systems.
	 *
	 * Key: System name (must be unique)
	 * Value: System instance
	 */
	TMap<FName, TSharedPtr<IAtlasSystem>> Systems;
};
