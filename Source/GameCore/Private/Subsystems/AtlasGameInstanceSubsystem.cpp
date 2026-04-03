#include "Subsystems/AtlasGameInstanceSubsystem.h"

#include "Interfaces/IAtlasSystem.h"
#include "Logging/AtlasLogMacros.h"
#include "Systems/AtlasSystemsRegistry.h"

/*
 * Called when GameInstance is initialized.
 *
 * Initializes all Atlas systems.
 */
void UAtlasGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ATLAS_LOG_CORE(Log, "AtlasGameInstanceSubsystem Initialize");

	RegisterSystems();
	InitializeSystems();
}

/*
 * Called during shutdown.
 *
 * Ensures all systems are properly cleaned up.
 */
void UAtlasGameInstanceSubsystem::Deinitialize()
{
	ATLAS_LOG_CORE(Log, "AtlasGameInstanceSubsystem Deinitialize");

	ShutdownSystems();

	Super::Deinitialize();
}

/*
 * Delegates system registration to the systems registry.
 */
void UAtlasGameInstanceSubsystem::RegisterSystems()
{
	ATLAS_LOG_CORE(Log, "Registering systems");

	FAtlasSystemsRegistry::Register(this);
}

/*
 * Calls Initialize() on all registered systems.
 */
void UAtlasGameInstanceSubsystem::InitializeSystems()
{
	for (const FName& SystemName : SystemOrder)
	{
		TSharedPtr<IAtlasSystem>* System = Systems.Find(SystemName);
		if (!System || !System->IsValid())
		{
			continue;
		}

		ATLAS_LOG_CORE(Log, "Initializing system: %s", *SystemName.ToString());
		(*System)->Initialize(this);
	}
}

/*
 * Calls Shutdown() on all systems and clears registry.
 */
void UAtlasGameInstanceSubsystem::ShutdownSystems()
{
	for (int32 Index = SystemOrder.Num() - 1; Index >= 0; --Index)
	{
		const FName& SystemName = SystemOrder[Index];
		TSharedPtr<IAtlasSystem>* System = Systems.Find(SystemName);
		if (!System || !System->IsValid())
		{
			continue;
		}

		ATLAS_LOG_CORE(Log, "Shutting down system: %s", *SystemName.ToString());
		(*System)->Shutdown();
	}

	Systems.Empty();
	SystemOrder.Empty();
}

/*
 * Registers a system instance into internal storage.
 */
void UAtlasGameInstanceSubsystem::RegisterSystem(TSharedPtr<IAtlasSystem> System)
{
	if (!System.IsValid())
	{
		return;
	}

	const FName SystemName = System->GetSystemName();
	if (Systems.Contains(SystemName))
	{
		ATLAS_LOG_CORE(Warning, "System already registered: %s", *SystemName.ToString());
		return;
	}

	Systems.Add(SystemName, System);
	SystemOrder.Add(SystemName);
}
