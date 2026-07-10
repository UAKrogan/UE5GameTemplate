#include "Data/AtlasInputConfigData.h"

const FPrimaryAssetType UAtlasInputConfigData::PrimaryAssetType = FPrimaryAssetType(TEXT("AtlasInputConfig"));

FPrimaryAssetId UAtlasInputConfigData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}
