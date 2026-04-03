#include "GameFramework.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameFrameworkModule"

void FGameFrameworkModule::StartupModule()
{
	ATLAS_LOG_FRAMEWORK(Warning, "Module Started");
}

void FGameFrameworkModule::ShutdownModule()
{
	ATLAS_LOG_FRAMEWORK(Warning, "Module Started");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameFrameworkModule, GameFramework)
