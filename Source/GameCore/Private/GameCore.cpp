#include "GameCore.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGameCoreModule, GameCore)

void FGameCoreModule::StartupModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameCore] Module Started"));
}

void FGameCoreModule::ShutdownModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameCore] Module Shutdown"));
}
