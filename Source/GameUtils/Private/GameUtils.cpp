#include "GameUtils.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameUtilsModule"

void FGameUtilsModule::StartupModule()
{
	ATLAS_LOG_UTILS(Log, "Module Started");
}

void FGameUtilsModule::ShutdownModule()
{
	ATLAS_LOG_UTILS(Log, "Module Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameUtilsModule, GameUtils)
