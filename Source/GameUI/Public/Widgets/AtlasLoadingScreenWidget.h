#pragma once

#include "Blueprint/UserWidget.h"
#include "Data/AtlasUITypes.h"
#include "AtlasLoadingScreenWidget.generated.h"

/*
 * Full-screen loading screen for the Loading layer.
 *
 * Not activatable — it must be addable at any time, including moments when
 * no valid local player exists (e.g. during travel).
 */
UCLASS(Blueprintable)
class GAMEUI_API UAtlasLoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeLoadingScreen(const FAtlasLoadingScreenConfig& InConfig);

	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	const FAtlasLoadingScreenConfig& GetConfig() const { return Config; }

	/*
	 * Optional progress feed (0..1) for widgets that display a bar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Atlas UI|Loading")
	void SetProgress(float InProgress);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI|Loading")
	void BP_OnLoadingScreenInitialized(const FAtlasLoadingScreenConfig& InConfig);

	UFUNCTION(BlueprintImplementableEvent, Category = "Atlas UI|Loading")
	void BP_OnProgressChanged(float InProgress);

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI|Loading")
	FAtlasLoadingScreenConfig Config;

	UPROPERTY(BlueprintReadOnly, Category = "Atlas UI|Loading")
	float Progress = 0.0f;
};
