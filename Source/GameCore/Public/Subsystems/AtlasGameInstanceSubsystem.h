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
 *   Initialize()  -> RegisterSystems() -> InitializeSystems()
 *   Deinitialize() -> ShutdownSystems()
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
	 * Retrieve a system by interface type.
	 *
	 * Usage examples:
	 *
	 *   // Example 1: inside Actor
	 *   UAtlasGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UAtlasGameInstanceSubsystem>();
	 *
	 *   TSharedPtr<IMySystem> System = Subsystem->GetSystem<IMySystem>();
	 *
	 *   // Example 2: inside Controller
	 *   if (UAtlasGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UAtlasGameInstanceSubsystem>())
	 *   {
	 *       if (TSharedPtr<IMySystem> System = Subsystem->GetSystem<IMySystem>())
	 *       {
	 *           // Use system
	 *       }
	 *   }
	 *
	 * Notes:
	 * - Returns nullptr if system not found
	 * - Interface must match the stored system type
	 */
	template<typename T>
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
	 * Register all systems here.
	 * This is the only place where systems are created.
	 */
	void RegisterSystems();

	/*
	 * Initialize all registered systems.
	 */
	void InitializeSystems();

	/*
	 * Shutdown all systems.
	 */
	void ShutdownSystems();

private:

	/*
	 * Storage for all runtime systems.
	 */
	TMap<FName, TSharedPtr<IAtlasSystem>> Systems;
};