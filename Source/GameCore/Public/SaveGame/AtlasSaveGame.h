#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGame/AtlasSaveGameTypes.h"
#include "AtlasSaveGame.generated.h"

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
