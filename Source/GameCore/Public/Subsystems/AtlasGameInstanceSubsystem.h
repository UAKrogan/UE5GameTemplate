#pragma once

#include "Interfaces/IAtlasSystem.h"
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
 * - Systems are registered externally (GameCore registry)
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
			if (Pair.Value.IsValid() && Pair.Value->SupportsInterface(T::InterfaceName()))
			{
				return StaticCastSharedPtr<T>(Pair.Value);
			}
		}

		return nullptr;
	}

private:
	/*
	 * Delegates system registration to the systems registry.
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

	/*
	 * Preserves deterministic lifecycle ordering.
	 *
	 * Systems are initialized in registration order and
	 * shut down in reverse registration order.
	 */
	TArray<FName> SystemOrder;
};
