#include "Game.h"
#include "Logging/AtlasLogMacros.h"

#define LOCTEXT_NAMESPACE "FGameModule"

// This code will execute after your module is loaded into memory;
// the exact timing is specified in the .uplugin file per-module
void FGameModule::StartupModule()
{
	ATLAS_LOG(Warning, "Game Started");

	//FModuleManager::Get().LoadModuleChecked("GameCore");
	//FModuleManager::Get().LoadModuleChecked("GameUI");
	//FModuleManager::Get().LoadModuleChecked("GameSystems");
	//FModuleManager::Get().LoadModuleChecked("GameUtils");
}

// This function may be called during shutdown to clean up your module.
// For modules that support dynamic reloading, we call this function before unloading the module.
void FGameModule::ShutdownModule()
{
	ATLAS_LOG(Warning, "Game Shutdown");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_PRIMARY_GAME_MODULE(FGameModule, Game, "Game");
