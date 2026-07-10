#include "Data/AtlasScreenRegistry.h"

#include "Data/AtlasScreenDefinition.h"
#include "Logging/AtlasLogMacros.h"

const UAtlasScreenDefinition* UAtlasScreenRegistry::FindScreen(FName ScreenId) const
{
	if (const TObjectPtr<UAtlasScreenDefinition>* RuntimeDefinition = RuntimeScreens.Find(ScreenId))
	{
		return *RuntimeDefinition;
	}

	if (const TSoftObjectPtr<UAtlasScreenDefinition>* SoftDefinition = Screens.Find(ScreenId))
	{
		return SoftDefinition->LoadSynchronous();
	}

	return nullptr;
}

void UAtlasScreenRegistry::RegisterScreen(FName ScreenId, UAtlasScreenDefinition* Definition)
{
	if (!ATLAS_ENSURE(Definition != nullptr))
	{
		return;
	}

	RuntimeScreens.Add(ScreenId, Definition);
	ATLAS_LOG_UI(Log, "Screen registered: %s", *ScreenId.ToString());
}

void UAtlasScreenRegistry::UnregisterScreen(FName ScreenId)
{
	if (RuntimeScreens.Remove(ScreenId) > 0)
	{
		ATLAS_LOG_UI(Log, "Screen unregistered: %s", *ScreenId.ToString());
	}
}
