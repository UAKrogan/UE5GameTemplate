#pragma once

#include "CoreMinimal.h"
#include "AtlasSaveHeader.generated.h"

/*
 * Header stored at the front of every Atlas binary snapshot payload.
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FAtlasSaveHeader
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Serialization")
	int32 Version = 1;

	UPROPERTY(VisibleAnywhere, Category = "Serialization")
	FDateTime Timestamp = FDateTime::UtcNow();

	bool IsSupportedVersion(int32 CurrentVersion) const
	{
		return Version > 0 && Version <= CurrentVersion;
	}
};
