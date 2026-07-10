#include "Settings/AtlasDeveloperSettings.h"

UAtlasDeveloperSettings::UAtlasDeveloperSettings()
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Atlas Framework");
}

const UAtlasDeveloperSettings* UAtlasDeveloperSettings::Get()
{
	return GetDefault<UAtlasDeveloperSettings>();
}

FName UAtlasDeveloperSettings::GetCategoryName() const
{
	return CategoryName;
}
