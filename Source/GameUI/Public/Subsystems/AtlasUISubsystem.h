#pragma once

#include "Data/AtlasUITypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasUISubsystem.generated.h"

class UAtlasActivatableWidget;
class UAtlasLoadingScreenWidget;
class UAtlasModalWidget;
class UAtlasRootWidget;
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

	UFUNCTION(BlueprintCallable, Category = "Atlas UI", meta = (AdvancedDisplay = "Context"))
	UAtlasActivatableWidget* PushScreen(FName ScreenId, UObject* Context = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	void PopScreen(FName ScreenId);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	void PopAllScreens();

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	UAtlasActivatableWidget* GetActiveScreen() const;

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	bool IsScreenActive(FName ScreenId) const;

	// ── HUD ──────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	void ShowHUD();

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	void HideHUD();

	// ── Modals ───────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal", meta = (AutoCreateRefTerm = "OnResult"))
	void ShowModal(FName ModalId, FAtlasModalPayload Payload, const FAtlasModalResultDelegate& OnResult);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Modal")
	void DismissModal();

	// ── Notifications ────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Notification")
	void ShowNotification(FAtlasNotificationPayload Payload);

	// ── Loading screen ───────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	void ShowLoadingScreen(FAtlasLoadingScreenConfig Config);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	void HideLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	bool IsLoadingScreenVisible() const;

	// ── Input mode ───────────────────────────────────────────────────

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
	const UAtlasScreenRegistry* ResolveScreenRegistry();
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void HandleScreenDeactivated(TWeakObjectPtr<UAtlasActivatableWidget> Widget);
	void UpdateInputModeFromScreens();
	void ApplyInputMode(EAtlasInputMode Mode);

	UPROPERTY(Transient)
	TObjectPtr<UAtlasRootWidget> RootWidget;

	UPROPERTY(Transient)
	TObjectPtr<const UAtlasScreenRegistry> ScreenRegistry;

	UPROPERTY(Transient)
	TObjectPtr<UAtlasLoadingScreenWidget> LoadingScreenWidget;

	TWeakObjectPtr<UAtlasModalWidget> ActiveModal;
	TArray<FAtlasActiveScreen> ActiveScreens;
	EAtlasInputMode CurrentInputMode = EAtlasInputMode::Game;
	FDelegateHandle PostLoadMapHandle;
};
