#include "Settings/AtlasUIDeveloperSettings.h"

UAtlasUIDeveloperSettings::UAtlasUIDeveloperSettings()
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Atlas UI");
}

const UAtlasUIDeveloperSettings* UAtlasUIDeveloperSettings::Get()
{
	return GetDefault<UAtlasUIDeveloperSettings>();
}
