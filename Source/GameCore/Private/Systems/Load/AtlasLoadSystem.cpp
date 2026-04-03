#include "Systems/Load/AtlasLoadSystem.h"

#include "Systems/Save/IAtlasSavable.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Logging/AtlasLogMacros.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

/*
 * Called during system initialization.
 */
void FAtlasLoadSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;

	ATLAS_LOG_CORE(Log, "LoadSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasLoadSystem::Shutdown()
{
	ATLAS_LOG_CORE(Log, "LoadSystem shutdown");
}

/*
 * Entry point for loading game state.
 */
void FAtlasLoadSystem::LoadGame()
{
	UWorld* World = OwningSubsystem ? OwningSubsystem->GetWorld() : nullptr;
	if (!World)
	{
		ATLAS_LOG_CORE(Error, "[LoadSystem] World is null");
		return;
	}

	ATLAS_LOG_CORE(Log, "[LoadSystem] Restoring actors");

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);

	for (AActor* Actor : Actors)
	{
		if (!Actor)
		{
			continue;
		}

		if (IAtlasSavable* Savable = Cast<IAtlasSavable>(Actor))
		{
			TArray<uint8> DummyData;
			FMemoryReader DummyArchive(DummyData);

			Savable->Load(DummyArchive);

			ATLAS_LOG_CORE(Log, "[LoadSystem] Loaded: %s", *Actor->GetName());
		}
	}
}
