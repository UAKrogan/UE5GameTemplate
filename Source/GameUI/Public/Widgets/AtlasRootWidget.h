#pragma once

#include "CommonUserWidget.h"
#include "Data/AtlasUITypes.h"
#include "AtlasRootWidget.generated.h"

class UCommonActivatableWidgetStack;
class UOverlay;

/*
 * Root widget owning the named UI layers.
 *
 * Layer Z-order (bottom to top): Game < Menu < Modal < Notification < Loading.
 *
 * A Blueprint subclass may bind the layer widgets by name (BindWidgetOptional);
 * when created from the raw C++ class the layer hierarchy is constructed
 * procedurally so the UI works with zero content assets.
 */
UCLASS(Blueprintable)
class GAMEUI_API UAtlasRootWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	UCommonActivatableWidgetStack* GetLayerStack(EAtlasUILayer Layer) const;

	UFUNCTION(BlueprintCallable, Category = "Atlas UI")
	UOverlay* GetLayerOverlay(EAtlasUILayer Layer) const;

protected:
	//~UUserWidget interface
	virtual void NativeOnInitialized() override;
	//~End of UUserWidget interface

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> GameLayer;

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI", meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayer;

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI", meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> NotificationLayer;

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI", meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> LoadingLayer;

private:
	void ConstructDefaultLayers();
};
