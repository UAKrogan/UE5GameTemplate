#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "AtlasUITypes.generated.h"

/*
 * Shared UI enums and payload structs used across the GameUI module.
 */

/*
 * Controls what receives input while a screen is active.
 */
UENUM(BlueprintType)
enum class EAtlasInputMode : uint8
{
	// Game input only, cursor hidden.
	Game,
	// UI input only, cursor visible.
	Menu,
	// Both game and UI input, cursor visible (e.g. in-game inventory).
	GameAndMenu,
	// Gamepad drives a virtual cursor over UI.
	GamepadCursor
};

/*
 * Named layers of the root widget, bottom to top.
 */
UENUM(BlueprintType)
enum class EAtlasUILayer : uint8
{
	// Gameplay HUD (activatable stack).
	Game,
	// Menus, pause, settings (activatable stack).
	Menu,
	// Modal dialogs (activatable stack).
	Modal,
	// Non-blocking toasts (overlay).
	Notification,
	// Loading screen, always on top (overlay).
	Loading
};

UENUM(BlueprintType)
enum class EAtlasNotificationType : uint8
{
	Info,
	Warning,
	Success
};

/*
 * Physical input device family currently driving the game, used for glyphs.
 */
UENUM(BlueprintType)
enum class EAtlasInputDevice : uint8
{
	KeyboardMouse,
	Xbox,
	PlayStation,
	Generic
};

/*
 * Text content for a modal dialog. The result callback travels separately
 * (FAtlasModalResultDelegate) because delegates are not UPROPERTY-safe
 * inside a payload struct.
 */
USTRUCT(BlueprintType)
struct FAtlasModalPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FText ConfirmLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	FText CancelLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modal")
	bool bShowCancel = true;
};

/*
 * Content and lifetime for a transient toast notification.
 */
USTRUCT(BlueprintType)
struct FAtlasNotificationPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	float DisplayDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	EAtlasNotificationType Type = EAtlasNotificationType::Info;
};

/*
 * Presentation options for a loading screen instance (background, tip,
 * optional progress bar). Fade timing is up to the widget Blueprint.
 */
USTRUCT(BlueprintType)
struct FAtlasLoadingScreenConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	TSoftObjectPtr<UTexture2D> BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	FText Tip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	bool bShowProgress = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float FadeInDuration = 0.25f;
};

/*
 * Result callback for modal dialogs (Blueprint-compatible).
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FAtlasModalResultDelegate, bool, bConfirmed);
