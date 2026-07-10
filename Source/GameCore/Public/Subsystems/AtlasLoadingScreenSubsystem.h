#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasLoadingScreenSubsystem.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAtlasLoadingScreenEvent);

/*
 * Manages loading screen visibility. Decoupled from level travel so any
 * system can request a loading screen independently.
 *
 * The widget class is resolved from UAtlasDeveloperSettings as a soft class
 * path and handled generically as UUserWidget — GameCore must not depend on
 * GameUI's widget types. The widget is added directly to the viewport
 * (bypassing the UI subsystem's layer stack) at a very high Z-order and
 * re-added after map loads so it stays visible across level travel.
 */
UCLASS()
class GAMECORE_API UAtlasLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	UFUNCTION(BlueprintCallable, Category = "Atlas|Loading")
	void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Loading")
	void HideLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Atlas|Loading")
	bool IsLoadingScreenVisible() const { return bVisible; }

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Loading")
	FAtlasLoadingScreenEvent OnLoadingScreenShown;

	UPROPERTY(BlueprintAssignable, Category = "Atlas|Loading")
	FAtlasLoadingScreenEvent OnLoadingScreenHidden;

private:
	void AddWidgetToViewport();
	void RemoveWidgetFromViewport();
	void HandlePostLoadMap(UWorld* LoadedWorld);

	// Above every other viewport widget, including the UI root.
	static constexpr int32 LoadingScreenZOrder = 10000;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoadingWidget;

	bool bVisible = false;
	FDelegateHandle PostLoadMapHandle;
};
