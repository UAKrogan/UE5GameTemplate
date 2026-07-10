#include "Data/AtlasAbilitySet.h"

const FPrimaryAssetType UAtlasAbilitySet::PrimaryAssetType = FPrimaryAssetType(TEXT("AtlasAbilitySet"));

FPrimaryAssetId UAtlasAbilitySet::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}
