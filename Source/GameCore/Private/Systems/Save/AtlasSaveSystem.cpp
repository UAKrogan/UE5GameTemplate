#include "Systems/Save/AtlasSaveSystem.h"

#include "Systems/Save/IAtlasSavable.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Logging/AtlasLogMacros.h"

#include "Serialization/BufferArchive.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

/*
 * Called during system initialization.
 */
void FAtlasSaveSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;

	ATLAS_LOG_CORE(Log, "SaveSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasSaveSystem::Shutdown()
{
	ATLAS_LOG_CORE(Log, "SaveSystem shutdown");
}

/*
 * Entry point for saving game state.
 */
void FAtlasSaveSystem::SaveGame()
{
	UWorld* World = OwningSubsystem ? OwningSubsystem->GetWorld() : nullptr;
	if (!World)
	{
		ATLAS_LOG_CORE(Error, "[SaveSystem] World is null");
		return;
	}

	ATLAS_LOG_CORE(Log, "[SaveSystem] Collecting savable actors");

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);

	int32 Count = 0;

	for (AActor* Actor : Actors)
	{
		if (!Actor)
		{
			continue;
		}

		if (IAtlasSavable* Savable = Cast<IAtlasSavable>(Actor))
		{
			FBufferArchive DummyArchive;

			Savable->Save(DummyArchive);

			Count++;

			ATLAS_LOG_CORE(Log, "[SaveSystem] Saved: %s", *Actor->GetName());
		}
	}

	ATLAS_LOG_CORE(Log, "[SaveSystem] Done. Count = %d", Count);
}
