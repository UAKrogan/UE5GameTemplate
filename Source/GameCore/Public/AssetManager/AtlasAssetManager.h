#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AtlasAssetManager.generated.h"

/*
 * Project asset manager.
 *
 * Owns the primary asset type identifiers used by the framework and is the
 * single place for startup asset scanning/preloading. Registered via
 * AssetManagerClassName in DefaultEngine.ini.
 */
UCLASS()
class GAMECORE_API UAtlasAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	/*
	 * Returns the asset manager singleton, checking that it is of this type.
	 */
	static UAtlasAssetManager& Get();

	/*
	 * Primary asset type identifiers. Data asset classes live in GameActors /
	 * GameUI; only the type names are owned here so any module can query them
	 * without new dependencies.
	 */
	static const FPrimaryAssetType PawnDataType;
	static const FPrimaryAssetType AbilitySetType;
	static const FPrimaryAssetType ScreenDefinitionType;
	static const FPrimaryAssetType InputConfigType;

protected:
	//~UAssetManager interface
	virtual void StartInitialLoading() override;
	//~End of UAssetManager interface
};
