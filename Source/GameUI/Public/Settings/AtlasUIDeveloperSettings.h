#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AtlasUIDeveloperSettings.generated.h"

class UAtlasInputGlyphData;
class UAtlasLoadingScreenWidget;
class UAtlasNotificationWidget;
class UAtlasRootWidget;
class UAtlasScreenRegistry;

/*
 * GameUI configuration. Appears under Project Settings -> Game -> Atlas UI.
 *
 * The screen registry and widget classes live here (not in GameCore's
 * UAtlasDeveloperSettings) because GameUI must not depend on GameCore.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Atlas UI"))
class GAMEUI_API UAtlasUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAtlasUIDeveloperSettings();

	static const UAtlasUIDeveloperSettings* Get();

	/*
	 * Root widget class; when unset the raw C++ UAtlasRootWidget is used and
	 * layers are constructed procedurally.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Root")
	TSoftClassPtr<UAtlasRootWidget> RootWidgetClass;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Root")
	int32 RootWidgetZOrder = 1000;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Screens")
	TSoftObjectPtr<UAtlasScreenRegistry> ScreenRegistry;

	/*
	 * Screen ID pushed by UAtlasUISubsystem::ShowHUD().
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Screens")
	FName HUDScreenId = TEXT("HUD");

	/*
	 * Loading screen widget class; when unset the raw C++
	 * UAtlasLoadingScreenWidget is used.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Loading")
	TSoftClassPtr<UAtlasLoadingScreenWidget> LoadingScreenWidgetClass;

	/*
	 * Notification (toast) widget class; when unset the raw C++
	 * UAtlasNotificationWidget is used.
	 */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Notifications")
	TSoftClassPtr<UAtlasNotificationWidget> NotificationWidgetClass;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Input Glyphs")
	TSoftObjectPtr<UAtlasInputGlyphData> InputGlyphData;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Input Glyphs")
	bool bAutoDetectInputDevice = true;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	bool bEnableGamepadNavigation = true;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	float AnalogNavigationRepeatDelay = 0.5f;
};
