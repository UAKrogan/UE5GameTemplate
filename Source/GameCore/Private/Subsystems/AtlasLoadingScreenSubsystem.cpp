#include "Subsystems/AtlasLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Logging/AtlasLogMacros.h"
#include "Settings/AtlasDeveloperSettings.h"
#include "UObject/UObjectGlobals.h"

void UAtlasLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Level travel removes all viewport widgets; re-add ours afterwards so
	// the loading screen survives until HideLoadingScreen() is called.
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UAtlasLoadingScreenSubsystem::HandlePostLoadMap);
}

void UAtlasLoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);

	RemoveWidgetFromViewport();
	LoadingWidget = nullptr;

	Super::Deinitialize();
}

void UAtlasLoadingScreenSubsystem::ShowLoadingScreen()
{
	if (bVisible)
	{
		return;
	}

	bVisible = true;
	AddWidgetToViewport();
	OnLoadingScreenShown.Broadcast();

	ATLAS_LOG_CORE(Log, "Loading screen shown");
}

void UAtlasLoadingScreenSubsystem::HideLoadingScreen()
{
	if (!bVisible)
	{
		return;
	}

	bVisible = false;
	RemoveWidgetFromViewport();
	OnLoadingScreenHidden.Broadcast();

	ATLAS_LOG_CORE(Log, "Loading screen hidden");
}

void UAtlasLoadingScreenSubsystem::AddWidgetToViewport()
{
	UGameViewportClient* Viewport = GetGameInstance() ? GetGameInstance()->GetGameViewportClient() : nullptr;
	if (Viewport == nullptr)
	{
		ATLAS_LOG_CORE(Warning, "Loading screen requested without a game viewport");
		return;
	}

	if (LoadingWidget == nullptr)
	{
		UClass* WidgetClass = UAtlasDeveloperSettings::Get()->LoadingScreenWidgetClass.TryLoadClass<UUserWidget>();
		if (WidgetClass == nullptr)
		{
			// No widget configured: visibility state and delegates still work
			// so a game-side listener can present its own loading UI.
			ATLAS_LOG_CORE(Verbose, "No LoadingScreenWidgetClass configured in Atlas Framework settings");
			return;
		}

		LoadingWidget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
		if (LoadingWidget == nullptr)
		{
			ATLAS_LOG_CORE(Warning, "Failed to create loading screen widget");
			return;
		}
	}

	Viewport->AddViewportWidgetContent(LoadingWidget->TakeWidget(), LoadingScreenZOrder);
}

void UAtlasLoadingScreenSubsystem::RemoveWidgetFromViewport()
{
	if (LoadingWidget == nullptr)
	{
		return;
	}

	if (UGameViewportClient* Viewport = GetGameInstance() ? GetGameInstance()->GetGameViewportClient() : nullptr)
	{
		Viewport->RemoveViewportWidgetContent(LoadingWidget->TakeWidget());
	}
}

void UAtlasLoadingScreenSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (bVisible)
	{
		AddWidgetToViewport();
	}
}
