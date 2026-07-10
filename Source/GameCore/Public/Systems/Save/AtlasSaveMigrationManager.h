#pragma once

#include "CoreMinimal.h"

struct FAtlasWorldSnapshot;

/*
 * Registry of save-format migrations, keyed by source version.
 *
 * When the binary format version is bumped, register a step that upgrades a
 * snapshot from version N to N+1:
 *
 *   FAtlasSaveMigrationManager::RegisterMigration(1, [](FAtlasWorldSnapshot& S)
 *   {
 *       // upgrade v1 -> v2 in place
 *       return true;
 *   });
 *
 * On load, FAtlasBinaryReader runs the steps sequentially (v1->v2, v2->v3,
 * ...) until the snapshot reaches the current version.
 */
class GAMECORE_API FAtlasSaveMigrationManager
{
public:
	using FAtlasMigrationFn = TFunction<bool(FAtlasWorldSnapshot&)>;

	static void RegisterMigration(int32 FromVersion, FAtlasMigrationFn Migration);

	/*
	 * Runs registered migrations sequentially. Returns false if any step is
	 * missing or fails; the snapshot must then be considered unusable.
	 */
	static bool MigrateSnapshot(FAtlasWorldSnapshot& Snapshot, int32 FromVersion, int32 ToVersion);

private:
	static TMap<int32, FAtlasMigrationFn>& GetMigrations();
};
