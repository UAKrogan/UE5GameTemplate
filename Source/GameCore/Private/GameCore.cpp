#include "GameCore.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameCoreModule"

void FGameCoreModule::StartupModule()
{
	ATLAS_LOG_CORE(Log, "Module Started");
}

void FGameCoreModule::ShutdownModule()
{
	ATLAS_LOG_CORE(Log, "Module Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameCoreModule, GameCore)
