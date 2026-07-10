#pragma once

#include "Engine/DataAsset.h"
#include "AtlasScreenRegistry.generated.h"

class UAtlasScreenDefinition;

/*
 * Registry mapping screen IDs to screen definitions. One per project,
 * referenced by UAtlasUIDeveloperSettings. Game Feature plugins contribute
 * additional screens at runtime via RegisterScreen/UnregisterScreen.
 */
UCLASS(BlueprintType)
class GAMEUI_API UAtlasScreenRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	/*
	 * Resolves a screen definition, loading the soft reference synchronously.
	 * Runtime-registered screens take precedence over authored entries.
	 */
	const UAtlasScreenDefinition* FindScreen(FName ScreenId) const;

	void RegisterScreen(FName ScreenId, UAtlasScreenDefinition* Definition);
	void UnregisterScreen(FName ScreenId);

	UPROPERTY(EditDefaultsOnly, Category = "Screens")
	TMap<FName, TSoftObjectPtr<UAtlasScreenDefinition>> Screens;

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UAtlasScreenDefinition>> RuntimeScreens;
};
