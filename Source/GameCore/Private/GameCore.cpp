#include "GameCore.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameCoreModule"

void FGameCoreModule::StartupModule()
{
	ATLAS_LOG_CORE(Warning, "Module Started");
}

void FGameCoreModule::ShutdownModule()
{
	ATLAS_LOG_CORE(Warning, "Module Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameCoreModule, GameCore)
