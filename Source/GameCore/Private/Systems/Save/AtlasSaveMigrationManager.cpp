#include "Systems/Save/AtlasSaveMigrationManager.h"

#include "Logging/AtlasLogMacros.h"
#include "Systems/WorldState/AtlasWorldSnapshotTypes.h"

void FAtlasSaveMigrationManager::RegisterMigration(int32 FromVersion, FAtlasMigrationFn Migration)
{
	if (!ATLAS_ENSURE(Migration))
	{
		return;
	}

	GetMigrations().Add(FromVersion, MoveTemp(Migration));
}

bool FAtlasSaveMigrationManager::MigrateSnapshot(FAtlasWorldSnapshot& Snapshot, int32 FromVersion, int32 ToVersion)
{
	if (FromVersion == ToVersion)
	{
		return true;
	}

	if (FromVersion > ToVersion)
	{
		ATLAS_LOG_CORE(Error, "Cannot migrate save data backwards: from=%d to=%d", FromVersion, ToVersion);
		return false;
	}

	for (int32 Version = FromVersion; Version < ToVersion; ++Version)
	{
		const FAtlasMigrationFn* Migration = GetMigrations().Find(Version);
		if (Migration == nullptr)
		{
			ATLAS_LOG_CORE(Error, "No save migration registered for version %d -> %d", Version, Version + 1);
			return false;
		}

		if (!(*Migration)(Snapshot))
		{
			ATLAS_LOG_CORE(Error, "Save migration failed: version %d -> %d", Version, Version + 1);
			return false;
		}

		ATLAS_LOG_CORE(Log, "Save data migrated: version %d -> %d", Version, Version + 1);
	}

	return true;
}

TMap<int32, FAtlasSaveMigrationManager::FAtlasMigrationFn>& FAtlasSaveMigrationManager::GetMigrations()
{
	static TMap<int32, FAtlasMigrationFn> Migrations;
	return Migrations;
}
