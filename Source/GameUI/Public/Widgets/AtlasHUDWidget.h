#pragma once

#include "Widgets/AtlasActivatableWidget.h"
#include "AtlasHUDWidget.generated.h"

class UNamedSlot;

/*
 * Root HUD widget for the Game layer.
 *
 * HUD content (health bars, minimap, objectives) is injected into named
 * element slots rather than hardcoded as children, so Game Feature plugins
 * can contribute HUD elements.
 */
UCLASS(Abstract, Blueprintable)
class GAMEUI_API UAtlasHUDWidget : public UAtlasActivatableWidget
{
	GENERATED_BODY()

public:
	UAtlasHUDWidget();

	/*
	 * Adds an element widget under the given slot name. Replaces any element
	 * already registered under that name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	void AddHUDElement(FName SlotName, UUserWidget* Element);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	void RemoveHUDElement(FName SlotName);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|HUD")
	UUserWidget* GetHUDElement(FName SlotName) const;

protected:
	/*
	 * Blueprint hook to place a newly added element in the HUD hierarchy.
	 * When not implemented, elements are tracked but not displayed — a HUD
	 * Blueprint must decide the actual layout.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI|HUD")
	void BP_OnHUDElementAdded(FName SlotName, UUserWidget* Element);

	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI|HUD")
	void BP_OnHUDElementRemoved(FName SlotName, UUserWidget* Element);

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UUserWidget>> HUDElements;
};
