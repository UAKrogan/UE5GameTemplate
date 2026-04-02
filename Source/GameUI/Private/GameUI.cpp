#include "GameUI.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGameUIModule, GameUI)

void FGameUIModule::StartupModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameUI] Module Started"));
}

void FGameUIModule::ShutdownModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameUI] Module Shutdown"));
}
