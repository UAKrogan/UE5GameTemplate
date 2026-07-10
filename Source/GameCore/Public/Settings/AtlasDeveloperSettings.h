#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AtlasDeveloperSettings.generated.h"

class UWorld;

/*
 * Project-wide Atlas framework configuration.
 *
 * Appears under Project Settings -> Game -> Atlas Framework.
 * Values are stored in Config/DefaultGame.ini under
 * [/Script/GameCore.AtlasDeveloperSettings].
 *
 * Cross-module asset references (pawn data, screen registry, loading screen
 * widget) are stored as soft paths, not typed pointers, because GameCore must
 * not depend on GameActors or GameUI.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Atlas Framework"))
class GAMECORE_API UAtlasDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAtlasDeveloperSettings();

	/*
	 * Convenience accessor for the config default object.
	 */
	static const UAtlasDeveloperSettings* Get();

	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

	/*
	 * Map shown first when the game boots (splash/initial load). Optional;
	 * when unset the flow goes straight to MainMenuMap.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	TSoftObjectPtr<UWorld> StartupMap;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	TSoftObjectPtr<UWorld> MainMenuMap;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	TSoftObjectPtr<UWorld> DefaultGameplayMap;

	/*
	 * Default UAtlasPawnData asset (GameActors) used when a spawned pawn has
	 * no explicit pawn data assigned.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Actors",
		meta = (AllowedClasses = "/Script/Engine.PrimaryDataAsset"))
	FSoftObjectPath DefaultPawnData;

	/*
	 * Widget class (GameUI UAtlasLoadingScreenWidget subclass) shown during
	 * level transitions and long async operations.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FSoftClassPath LoadingScreenWidgetClass;

	/*
	 * UAtlasScreenRegistry asset (GameUI) mapping screen IDs to definitions.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FSoftObjectPath ScreenRegistry;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Loading")
	float LoadingScreenFadeInDuration = 0.25f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Loading")
	float LoadingScreenFadeOutDuration = 0.25f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	FString DefaultSaveSlotName = TEXT("PlayerSave_01");

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	bool bEnableAutosave = true;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Save",
		meta = (ClampMin = "10.0", Units = "s"))
	float AutosaveIntervalSeconds = 300.0f;

	/*
	 * When true, a checkpoint save is requested before every level transition.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	bool bSaveCheckpointOnTravel = false;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Features")
	bool bAutoActivateGameFeatures = true;
};
