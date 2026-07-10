#include "Data/AtlasPawnData.h"

const FPrimaryAssetType UAtlasPawnData::PrimaryAssetType = FPrimaryAssetType(TEXT("AtlasPawnData"));

FPrimaryAssetId UAtlasPawnData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}
