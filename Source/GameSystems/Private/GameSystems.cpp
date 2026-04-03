#include "GameSystems.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameSystemsModule"

void FGameSystemsModule::StartupModule()
{
	ATLAS_LOG_SYSTEMS(Warning, "Module Started");
}

void FGameSystemsModule::ShutdownModule()
{
	ATLAS_LOG_SYSTEMS(Warning, "Module Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameSystemsModule, GameSystems)
