#pragma once

#include "Data/AtlasUITypes.h"
#include "Engine/DataAsset.h"
#include "AtlasScreenDefinition.generated.h"

class UAtlasActivatableWidget;

/*
 * Defines a single UI screen: which widget class to spawn, which layer to
 * push it on, and what input mode it requests.
 *
 * Primary asset type: AtlasScreenDefinition.
 */
UCLASS(BlueprintType)
class GAMEUI_API UAtlasScreenDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	//~UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//~End of UPrimaryDataAsset interface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	FName ScreenId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	TSoftClassPtr<UAtlasActivatableWidget> WidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	EAtlasUILayer TargetLayer = EAtlasUILayer::Menu;

	/*
	 * Overrides the widget's own requested input mode when set.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	EAtlasInputMode RequestedInputMode = EAtlasInputMode::Menu;

	/*
	 * When true, pushing the screen while an instance is already on its layer
	 * is a no-op.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	bool bSingleInstance = true;
};
