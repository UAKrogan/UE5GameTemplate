#include "AssetManager/AtlasAssetManager.h"

#include "Logging/AtlasLogMacros.h"

const FPrimaryAssetType UAtlasAssetManager::PawnDataType = FPrimaryAssetType(TEXT("AtlasPawnData"));
const FPrimaryAssetType UAtlasAssetManager::AbilitySetType = FPrimaryAssetType(TEXT("AtlasAbilitySet"));
const FPrimaryAssetType UAtlasAssetManager::ScreenDefinitionType = FPrimaryAssetType(TEXT("AtlasScreenDefinition"));
const FPrimaryAssetType UAtlasAssetManager::InputConfigType = FPrimaryAssetType(TEXT("AtlasInputConfig"));

UAtlasAssetManager& UAtlasAssetManager::Get()
{
	UAtlasAssetManager* Manager = Cast<UAtlasAssetManager>(GEngine->AssetManager);
	ATLAS_ENSURE_MSG(Manager != nullptr,
		"AssetManagerClassName is not set to /Script/GameCore.AtlasAssetManager in DefaultEngine.ini");

	if (Manager == nullptr)
	{
		// Fatal misconfiguration: fall back so callers get a usable manager,
		// but the template will not resolve Atlas primary asset types.
		Manager = NewObject<UAtlasAssetManager>();
	}

	return *Manager;
}

void UAtlasAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	ATLAS_LOG_CORE(Log, "Asset manager initialized (%s)", *GetClass()->GetName());
}
