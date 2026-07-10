#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGame/AtlasSaveGameTypes.h"
#include "AtlasSaveGame.generated.h"

/*
 * USaveGame wrapper around the flat key/value snapshot, for interop with
 * engine SaveGame tooling. The world-state pipeline uses the Atlas binary
 * format directly and does not go through this class.
 */
UCLASS()
class GAMECORE_API UAtlasSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	void SetSnapshot(const FAtlasSaveGameSnapshot& InSnapshot)
	{
		Snapshot = InSnapshot;
	}

	const FAtlasSaveGameSnapshot& GetSnapshot() const
	{
		return Snapshot;
	}

private:
	UPROPERTY()
	FAtlasSaveGameSnapshot Snapshot;
};
