#pragma once

#include "Engine/DataAsset.h"
#include "AtlasPawnData.generated.h"

class APawn;
class UAtlasAbilitySet;
class UAtlasCameraConfig;
class UAtlasInputConfigData;

/*
 * Per-pawn-type configuration: everything needed to set up a pawn instance
 * without hardcoding in actor classes.
 *
 * Primary asset type: AtlasPawnData.
 */
UCLASS(BlueprintType)
class GAMEACTORS_API UAtlasPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	//~UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~End of UPrimaryDataAsset interface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSoftClassPtr<APawn> PawnClass;

	// Granted on possession by the pawn extension component.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSoftObjectPtr<UAtlasAbilitySet>> AbilitySets;

	// Applied on possession by the input extension component.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TArray<TSoftObjectPtr<UAtlasInputConfigData>> InputConfigs;

	// Default camera setup, applied by the camera extension component when
	// one is present on the pawn. Optional.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSoftObjectPtr<UAtlasCameraConfig> CameraConfig;
};
