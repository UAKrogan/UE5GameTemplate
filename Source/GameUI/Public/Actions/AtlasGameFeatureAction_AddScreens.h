#pragma once

#include "GameFeatureAction.h"
#include "AtlasGameFeatureAction_AddScreens.generated.h"

class UAtlasScreenDefinition;

USTRUCT()
struct FAtlasScreenRegistration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Screen")
	FName ScreenId;

	UPROPERTY(EditAnywhere, Category = "Screen")
	TSoftObjectPtr<UAtlasScreenDefinition> Definition;
};

/*
 * Registers screen definitions with the UI subsystem's screen registry
 * while the feature is active. Active screens are popped on deactivation.
 */
UCLASS(meta = (DisplayName = "Atlas: Add Screens"))
class GAMEUI_API UAtlasGameFeatureAction_AddScreens : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureActivating() override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category = "Screens")
	TArray<FAtlasScreenRegistration> Screens;
};
