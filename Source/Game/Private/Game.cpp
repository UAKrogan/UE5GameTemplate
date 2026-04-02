#include "Game.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FGameModule, Game, "Game");

// This code will execute after your module is loaded into memory;
// the exact timing is specified in the .uplugin file per-module
void FGameModule::StartupModule()
{
	UE_LOG(LogInit, Warning, TEXT("[Game] Module Started"));

	//FModuleManager::Get().LoadModuleChecked("GameCore");
	//FModuleManager::Get().LoadModuleChecked("GameUI");
	//FModuleManager::Get().LoadModuleChecked("GameSystems");
	//FModuleManager::Get().LoadModuleChecked("GameUtils");
}

// This function may be called during shutdown to clean up your module.
// For modules that support dynamic reloading, we call this function before unloading the module.
void FGameModule::ShutdownModule()
{
	UE_LOG(LogInit, Warning, TEXT("[Game] Module Shutdown"));
}
