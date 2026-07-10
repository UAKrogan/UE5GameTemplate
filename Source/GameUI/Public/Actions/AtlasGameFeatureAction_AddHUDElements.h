#pragma once

#include "GameFeatureAction.h"
#include "AtlasGameFeatureAction_AddHUDElements.generated.h"

class UUserWidget;

USTRUCT()
struct FAtlasHUDElementEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "HUD")
	FName SlotName;

	UPROPERTY(EditAnywhere, Category = "HUD")
	TSoftClassPtr<UUserWidget> WidgetClass;
};

/*
 * Injects widgets into named slots of the active HUD while the feature is
 * active. If no HUD is on screen at activation time, elements are added the
 * next time a HUD screen is pushed.
 */
UCLASS(meta = (DisplayName = "Atlas: Add HUD Elements"))
class GAMEUI_API UAtlasGameFeatureAction_AddHUDElements : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureActivating() override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category = "HUD")
	TArray<FAtlasHUDElementEntry> Elements;

private:
	void AddElementsToActiveHUDs();
	void RemoveElementsFromActiveHUDs();

	UFUNCTION()
	void HandleScreenPushed(FName ScreenId);

	TArray<TWeakObjectPtr<class UAtlasUISubsystem>> BoundSubsystems;
};
