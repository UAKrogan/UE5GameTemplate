#include "GameUtils.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGameUtilsModule, GameUtils)

void FGameUtilsModule::StartupModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameUtils] Module Started"));
}

void FGameUtilsModule::ShutdownModule()
{
	UE_LOG(LogInit, Warning, TEXT("[GameUtils] Module Shutdown"));
}
