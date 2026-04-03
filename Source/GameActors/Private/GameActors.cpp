#include "GameActors.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameActorsModule"

void FGameActorsModule::StartupModule()
{
	ATLAS_LOG_ACTORS(Log, "Module Started");
}

void FGameActorsModule::ShutdownModule()
{
	ATLAS_LOG_ACTORS(Log, "Module Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameActorsModule, GameActors)
