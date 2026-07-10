#pragma once

#include "CommonActivatableWidget.h"
#include "Data/AtlasUITypes.h"
#include "AtlasActivatableWidget.generated.h"

/*
 * Base class for all Atlas navigable screens.
 *
 * Never use bare UUserWidget for navigable screens — Common UI activation,
 * focus, and back handling all flow through this class.
 */
UCLASS(Abstract, Blueprintable)
class GAMEUI_API UAtlasActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/*
	 * Called by the UI subsystem right after the widget is pushed, before
	 * activation, with optional caller-provided context.
	 */
	virtual void InitializeWithContext(UObject* Context);

	/*
	 * Blueprint hook mirroring InitializeWithContext.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI", meta = (DisplayName = "On Initialized With Context"))
	void BP_OnInitializedWithContext(UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	virtual EAtlasInputMode GetRequestedInputMode() const;

	/*
	 * Whether back navigation (gamepad B / Escape) should pop this screen.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	virtual bool GetRequestsDismissOnBack() const;

protected:
	//~UCommonActivatableWidget interface
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	//~End of UCommonActivatableWidget interface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atlas UI")
	EAtlasInputMode RequestedInputMode = EAtlasInputMode::Menu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atlas UI")
	bool bDismissOnBack = true;
};
