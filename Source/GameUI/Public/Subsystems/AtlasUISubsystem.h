#pragma once

#include "Data/AtlasUITypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasUISubsystem.generated.h"

class UAtlasActivatableWidget;
class UAtlasHUDWidget;
class UAtlasLoadingScreenWidget;
class UAtlasModalWidget;
class UAtlasRootWidget;
class UAtlasScreenDefinition;
class UAtlasScreenRegistry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasScreenEvent, FName, ScreenId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasInputModeChanged, EAtlasInputMode, NewMode);

/*
 * Central UI runtime service.
 *
 * Owns the layered root widget, resolves screen IDs through the screen
 * registry, and manages input mode. Callers push/pop screen IDs; the
 * subsystem resolves widget classes — screens are never hardcoded in C++.
 */
UCLASS()
class GAMEUI_API UAtlasUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ── Screens ──────────────────────────────────────────────────────

	/*
	 * Resolves the screen definition from the registry, creates the widget
	 * on the definition's layer, and applies its input mode. The optional
	 * Context is handed to the widget via InitializeWithContext before
	 * activation. Returns null on failure (unknown ID, unloadable class,
	 * single-instance screen already active).
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI", meta = (AdvancedDisplay = "Context"))
	UAtlasActivatableWidget* PushScreen(FName ScreenId, UObject* Context = nullptr);

	/*
	 * Pops the top-most active instance of the given screen ID.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	void PopScreen(FName ScreenId);

	/*
	 * Pops every tracked screen on every layer (e.g. before level travel).
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	void PopAllScreens();

	/*
	 * The top-most active screen: highest layer wins, then most recent push.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	UAtlasActivatableWidget* GetActiveScreen() const;

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	bool IsScreenActive(FName ScreenId) const;

	/*
	 * Runtime screen registration, used by Game Feature actions to
	 * contribute screens without touching the authored registry asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	void RegisterScreen(FName ScreenId, UAtlasScreenDefinition* Definition);

	/*
	 * Unregisters a runtime screen; pops it first if currently active.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	void UnregisterScreen(FName ScreenId);

	// ── HUD ──────────────────────────────────────────────────────────

	/*
	 * Pushes/pops the screen registered under the HUDScreenId configured in
	 * Atlas UI settings (default "HUD").
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	void ShowHUD();

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	void HideHUD();

	/*
	 * The active HUD screen cast to UAtlasHUDWidget, or null when no HUD is
	 * on screen. Used by feature actions to inject HUD elements.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	UAtlasHUDWidget* GetActiveHUDWidget() const;

	// ── Modals ───────────────────────────────────────────────────────

	/*
	 * Pushes the modal screen (must resolve to a UAtlasModalWidget), hands
	 * it the payload, and fires OnResult(bConfirmed) exactly once when the
	 * dialog is confirmed, cancelled, or dismissed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal", meta = (AutoCreateRefTerm = "OnResult"))
	void ShowModal(FName ModalId, FAtlasModalPayload Payload, const FAtlasModalResultDelegate& OnResult);

	/*
	 * Cancels the active modal (its result delegate fires with false).
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal")
	void DismissModal();

	// ── Notifications ────────────────────────────────────────────────

	/*
	 * Shows a non-blocking toast on the Notification layer; auto-removed
	 * after the payload's display duration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Notification")
	void ShowNotification(FAtlasNotificationPayload Payload);

	// ── Loading screen ───────────────────────────────────────────────

	/*
	 * Shows a loading screen on the root widget's Loading layer. For level
	 * travel use UAtlasLoadingScreenSubsystem (GameCore) instead — this one
	 * lives inside the root widget and does not survive travel by itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	void ShowLoadingScreen(FAtlasLoadingScreenConfig Config);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	void HideLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	bool IsLoadingScreenVisible() const;

	// ── Input mode ───────────────────────────────────────────────────

	/*
	 * Applies an input mode to the first local player controller and
	 * broadcasts OnInputModeChanged. Usually driven automatically by the
	 * screen stack — call directly only for special cases.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Input")
	void SetInputMode(EAtlasInputMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Input")
	EAtlasInputMode GetCurrentInputMode() const { return CurrentInputMode; }

	// ── Events ───────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Atlas UI")
	FAtlasScreenEvent OnScreenPushed;

	UPROPERTY(BlueprintAssignable, Category = "Atlas UI")
	FAtlasScreenEvent OnScreenPopped;

	UPROPERTY(BlueprintAssignable, Category = "Atlas UI")
	FAtlasInputModeChanged OnInputModeChanged;

protected:
	/*
	 * Resolves the root widget class from UAtlasUIDeveloperSettings; override
	 * to provide a custom root.
	 */
	virtual UAtlasRootWidget* CreateRootWidget();

private:
	struct FAtlasActiveScreen
	{
		FName ScreenId;
		EAtlasUILayer Layer = EAtlasUILayer::Menu;
		EAtlasInputMode InputMode = EAtlasInputMode::Menu;
		TWeakObjectPtr<UAtlasActivatableWidget> Widget;
	};

	bool EnsureRootWidget();
	UAtlasScreenRegistry* ResolveScreenRegistry();
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void HandleScreenDeactivated(TWeakObjectPtr<UAtlasActivatableWidget> Widget);
	void UpdateInputModeFromScreens();
	void ApplyInputMode(EAtlasInputMode Mode);

	UPROPERTY(Transient)
	TObjectPtr<UAtlasRootWidget> RootWidget;

	UPROPERTY(Transient)
	TObjectPtr<UAtlasScreenRegistry> ScreenRegistry;

	UPROPERTY(Transient)
	TObjectPtr<UAtlasLoadingScreenWidget> LoadingScreenWidget;

	TWeakObjectPtr<UAtlasModalWidget> ActiveModal;
	TArray<FAtlasActiveScreen> ActiveScreens;
	EAtlasInputMode CurrentInputMode = EAtlasInputMode::Game;
	FDelegateHandle PostLoadMapHandle;
};
