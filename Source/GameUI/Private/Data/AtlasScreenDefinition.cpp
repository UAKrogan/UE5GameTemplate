#include "Data/AtlasScreenDefinition.h"

const FPrimaryAssetType UAtlasScreenDefinition::PrimaryAssetType = FPrimaryAssetType(TEXT("AtlasScreenDefinition"));

FPrimaryAssetId UAtlasScreenDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, ScreenId.IsNone() ? GetFName() : ScreenId);
}
