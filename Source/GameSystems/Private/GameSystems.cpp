#include "GameSystems.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGameSystemsModule, GameSystems)

void FGameSystemsModule::StartupModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameSystems] Module Started"));
}

void FGameSystemsModule::ShutdownModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameSystems] Module Shutdown"));
}
