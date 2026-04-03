#include "GameUI.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameUIModule"

void FGameUIModule::StartupModule()
{
	ATLAS_LOG_UI(Log, "Module Started");
}

void FGameUIModule::ShutdownModule()
{
	ATLAS_LOG_UI(Log, "Module Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameUIModule, GameUI)
