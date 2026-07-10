#include "Subsystems/AtlasUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Data/AtlasScreenDefinition.h"
#include "Data/AtlasScreenRegistry.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Logging/AtlasLogMacros.h"
#include "Settings/AtlasUIDeveloperSettings.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/AtlasLoadingScreenWidget.h"
#include "Widgets/AtlasModalWidget.h"
#include "Widgets/AtlasNotificationWidget.h"
#include "Widgets/AtlasRootWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UAtlasUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Level travel removes all viewport widgets; re-add the root afterwards
	// so layers (and any surviving loading screen) come back automatically.
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UAtlasUISubsystem::HandlePostLoadMap);

	ATLAS_LOG_UI(Log, "UISubsystem initialized");
}

void UAtlasUISubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);

	PopAllScreens();

	if (RootWidget != nullptr)
	{
		RootWidget->RemoveFromParent();
		RootWidget = nullptr;
	}

	ATLAS_LOG_UI(Log, "UISubsystem shutdown");

	Super::Deinitialize();
}

UAtlasActivatableWidget* UAtlasUISubsystem::PushScreen(FName ScreenId, UObject* Context)
{
	if (!EnsureRootWidget())
	{
		return nullptr;
	}

	const UAtlasScreenRegistry* Registry = ResolveScreenRegistry();
	if (Registry == nullptr)
	{
		ATLAS_LOG_UI(Warning, "PushScreen(%s) failed: no screen registry configured in Atlas UI settings", *ScreenId.ToString());
		return nullptr;
	}

	const UAtlasScreenDefinition* Definition = Registry->FindScreen(ScreenId);
	if (Definition == nullptr)
	{
		ATLAS_LOG_UI(Warning, "PushScreen(%s) failed: screen not found in registry", *ScreenId.ToString());
		return nullptr;
	}

	if (Definition->bSingleInstance && IsScreenActive(ScreenId))
	{
		ATLAS_LOG_UI(Verbose, "PushScreen(%s) skipped: single-instance screen already active", *ScreenId.ToString());
		return nullptr;
	}

	UClass* WidgetClass = Definition->WidgetClass.LoadSynchronous();
	if (WidgetClass == nullptr)
	{
		ATLAS_LOG_UI(Warning, "PushScreen(%s) failed: widget class could not be loaded", *ScreenId.ToString());
		return nullptr;
	}

	UCommonActivatableWidgetStack* Stack = RootWidget->GetLayerStack(Definition->TargetLayer);
	if (Stack == nullptr)
	{
		return nullptr;
	}

	UAtlasActivatableWidget* Widget = Stack->AddWidget<UAtlasActivatableWidget>(
		WidgetClass,
		[Context](UAtlasActivatableWidget& NewWidget)
		{
			NewWidget.InitializeWithContext(Context);
		});
	if (Widget == nullptr)
	{
		ATLAS_LOG_UI(Warning, "PushScreen(%s) failed: stack did not create the widget", *ScreenId.ToString());
		return nullptr;
	}

	FAtlasActiveScreen& Entry = ActiveScreens.AddDefaulted_GetRef();
	Entry.ScreenId = ScreenId;
	Entry.Layer = Definition->TargetLayer;
	Entry.InputMode = Definition->RequestedInputMode;
	Entry.Widget = Widget;

	Widget->OnDeactivated().AddWeakLambda(this,
		[this, WidgetPtr = TWeakObjectPtr<UAtlasActivatableWidget>(Widget)]()
		{
			HandleScreenDeactivated(WidgetPtr);
		});

	UpdateInputModeFromScreens();
	OnScreenPushed.Broadcast(ScreenId);

	ATLAS_LOG_UI(Log, "Screen pushed: %s", *ScreenId.ToString());

	return Widget;
}

void UAtlasUISubsystem::PopScreen(FName ScreenId)
{
	// Iterate backwards so the top-most instance of the screen pops first.
	for (int32 Index = ActiveScreens.Num() - 1; Index >= 0; --Index)
	{
		if (ActiveScreens[Index].ScreenId != ScreenId)
		{
			continue;
		}

		if (UAtlasActivatableWidget* Widget = ActiveScreens[Index].Widget.Get())
		{
			if (UCommonActivatableWidgetStack* Stack = RootWidget ? RootWidget->GetLayerStack(ActiveScreens[Index].Layer) : nullptr)
			{
				// Removal deactivates the widget; bookkeeping happens in
				// HandleScreenDeactivated.
				Stack->RemoveWidget(*Widget);
				return;
			}
		}

		ActiveScreens.RemoveAt(Index);
		return;
	}
}

void UAtlasUISubsystem::PopAllScreens()
{
	TArray<FAtlasActiveScreen> Screens = MoveTemp(ActiveScreens);
	ActiveScreens.Reset();

	for (int32 Index = Screens.Num() - 1; Index >= 0; --Index)
	{
		if (UAtlasActivatableWidget* Widget = Screens[Index].Widget.Get())
		{
			if (UCommonActivatableWidgetStack* Stack = RootWidget ? RootWidget->GetLayerStack(Screens[Index].Layer) : nullptr)
			{
				Stack->RemoveWidget(*Widget);
			}
		}

		OnScreenPopped.Broadcast(Screens[Index].ScreenId);
	}

	UpdateInputModeFromScreens();
}

UAtlasActivatableWidget* UAtlasUISubsystem::GetActiveScreen() const
{
	// Highest layer wins; within a layer, the most recently pushed wins.
	const FAtlasActiveScreen* Top = nullptr;
	for (const FAtlasActiveScreen& Screen : ActiveScreens)
	{
		if (!Screen.Widget.IsValid())
		{
			continue;
		}

		if (Top == nullptr || static_cast<uint8>(Screen.Layer) >= static_cast<uint8>(Top->Layer))
		{
			Top = &Screen;
		}
	}

	return Top != nullptr ? Top->Widget.Get() : nullptr;
}

bool UAtlasUISubsystem::IsScreenActive(FName ScreenId) const
{
	for (const FAtlasActiveScreen& Screen : ActiveScreens)
	{
		if (Screen.ScreenId == ScreenId && Screen.Widget.IsValid())
		{
			return true;
		}
	}

	return false;
}

void UAtlasUISubsystem::ShowHUD()
{
	PushScreen(UAtlasUIDeveloperSettings::Get()->HUDScreenId);
}

void UAtlasUISubsystem::HideHUD()
{
	PopScreen(UAtlasUIDeveloperSettings::Get()->HUDScreenId);
}

void UAtlasUISubsystem::ShowModal(FName ModalId, FAtlasModalPayload Payload, const FAtlasModalResultDelegate& OnResult)
{
	UAtlasActivatableWidget* Widget = PushScreen(ModalId);
	UAtlasModalWidget* Modal = Cast<UAtlasModalWidget>(Widget);
	if (Modal == nullptr)
	{
		ATLAS_LOG_UI(Warning, "ShowModal(%s): screen is not a UAtlasModalWidget", *ModalId.ToString());
		return;
	}

	Modal->SetPayload(Payload, OnResult);
	ActiveModal = Modal;
}

void UAtlasUISubsystem::DismissModal()
{
	if (UAtlasModalWidget* Modal = ActiveModal.Get())
	{
		Modal->Cancel();
	}

	ActiveModal.Reset();
}

void UAtlasUISubsystem::ShowNotification(FAtlasNotificationPayload Payload)
{
	if (!EnsureRootWidget())
	{
		return;
	}

	UOverlay* Layer = RootWidget->GetLayerOverlay(EAtlasUILayer::Notification);
	if (Layer == nullptr)
	{
		return;
	}

	UClass* WidgetClass = UAtlasUIDeveloperSettings::Get()->NotificationWidgetClass.LoadSynchronous();
	if (WidgetClass == nullptr)
	{
		WidgetClass = UAtlasNotificationWidget::StaticClass();
	}

	UAtlasNotificationWidget* Notification = CreateWidget<UAtlasNotificationWidget>(GetGameInstance(), WidgetClass);
	if (Notification == nullptr)
	{
		return;
	}

	Notification->InitializeNotification(Payload);

	UOverlaySlot* OverlaySlot = Layer->AddChildToOverlay(Notification);
	OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
	OverlaySlot->SetVerticalAlignment(VAlign_Fill);

	const float Duration = FMath::Max(Payload.DisplayDurationSeconds, 0.1f);
	FTimerHandle DismissHandle;
	GetGameInstance()->GetTimerManager().SetTimer(
		DismissHandle,
		FTimerDelegate::CreateWeakLambda(Notification, [Notification]()
		{
			Notification->RemoveFromParent();
		}),
		Duration,
		/*bLoop*/ false);
}

void UAtlasUISubsystem::ShowLoadingScreen(FAtlasLoadingScreenConfig Config)
{
	if (!EnsureRootWidget())
	{
		return;
	}

	if (IsLoadingScreenVisible())
	{
		return;
	}

	UOverlay* Layer = RootWidget->GetLayerOverlay(EAtlasUILayer::Loading);
	if (Layer == nullptr)
	{
		return;
	}

	UClass* WidgetClass = UAtlasUIDeveloperSettings::Get()->LoadingScreenWidgetClass.LoadSynchronous();
	if (WidgetClass == nullptr)
	{
		WidgetClass = UAtlasLoadingScreenWidget::StaticClass();
	}

	LoadingScreenWidget = CreateWidget<UAtlasLoadingScreenWidget>(GetGameInstance(), WidgetClass);
	if (LoadingScreenWidget == nullptr)
	{
		return;
	}

	LoadingScreenWidget->InitializeLoadingScreen(Config);

	UOverlaySlot* OverlaySlot = Layer->AddChildToOverlay(LoadingScreenWidget);
	OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
	OverlaySlot->SetVerticalAlignment(VAlign_Fill);

	ATLAS_LOG_UI(Log, "Loading screen shown");
}

void UAtlasUISubsystem::HideLoadingScreen()
{
	if (LoadingScreenWidget != nullptr)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;

		ATLAS_LOG_UI(Log, "Loading screen hidden");
	}
}

bool UAtlasUISubsystem::IsLoadingScreenVisible() const
{
	return LoadingScreenWidget != nullptr && LoadingScreenWidget->GetParent() != nullptr;
}

void UAtlasUISubsystem::SetInputMode(EAtlasInputMode Mode)
{
	if (Mode == CurrentInputMode)
	{
		return;
	}

	CurrentInputMode = Mode;
	ApplyInputMode(Mode);
	OnInputModeChanged.Broadcast(Mode);
}

UAtlasRootWidget* UAtlasUISubsystem::CreateRootWidget()
{
	UClass* RootClass = UAtlasUIDeveloperSettings::Get()->RootWidgetClass.LoadSynchronous();
	if (RootClass == nullptr)
	{
		RootClass = UAtlasRootWidget::StaticClass();
	}

	return CreateWidget<UAtlasRootWidget>(GetGameInstance(), RootClass);
}

bool UAtlasUISubsystem::EnsureRootWidget()
{
	if (RootWidget != nullptr && RootWidget->GetParent() == nullptr && !RootWidget->IsInViewport())
	{
		RootWidget->AddToViewport(UAtlasUIDeveloperSettings::Get()->RootWidgetZOrder);
	}

	if (RootWidget != nullptr)
	{
		return true;
	}

	// Root creation needs a world for the widget to live in; callers may hit
	// this before any map is loaded, in which case pushing UI must wait.
	if (GetGameInstance() == nullptr || GetGameInstance()->GetWorld() == nullptr)
	{
		ATLAS_LOG_UI(Warning, "Root widget requested before a world exists");
		return false;
	}

	RootWidget = CreateRootWidget();
	if (RootWidget == nullptr)
	{
		ATLAS_LOG_UI(Warning, "Failed to create root widget");
		return false;
	}

	RootWidget->AddToViewport(UAtlasUIDeveloperSettings::Get()->RootWidgetZOrder);

	ATLAS_LOG_UI(Log, "Root widget created and added to viewport");

	return true;
}

const UAtlasScreenRegistry* UAtlasUISubsystem::ResolveScreenRegistry()
{
	if (ScreenRegistry == nullptr)
	{
		ScreenRegistry = UAtlasUIDeveloperSettings::Get()->ScreenRegistry.LoadSynchronous();
	}

	return ScreenRegistry;
}

void UAtlasUISubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (RootWidget != nullptr && !RootWidget->IsInViewport())
	{
		RootWidget->AddToViewport(UAtlasUIDeveloperSettings::Get()->RootWidgetZOrder);

		ATLAS_LOG_UI(Log, "Root widget re-added to viewport after level travel");
	}
}

void UAtlasUISubsystem::HandleScreenDeactivated(TWeakObjectPtr<UAtlasActivatableWidget> Widget)
{
	for (int32 Index = ActiveScreens.Num() - 1; Index >= 0; --Index)
	{
		if (ActiveScreens[Index].Widget != Widget)
		{
			continue;
		}

		const FName ScreenId = ActiveScreens[Index].ScreenId;
		ActiveScreens.RemoveAt(Index);

		UpdateInputModeFromScreens();
		OnScreenPopped.Broadcast(ScreenId);

		ATLAS_LOG_UI(Log, "Screen popped: %s", *ScreenId.ToString());
		return;
	}
}

void UAtlasUISubsystem::UpdateInputModeFromScreens()
{
	// The top-most valid screen (highest layer, most recent) owns input mode.
	const FAtlasActiveScreen* Top = nullptr;
	for (const FAtlasActiveScreen& Screen : ActiveScreens)
	{
		if (!Screen.Widget.IsValid())
		{
			continue;
		}

		if (Top == nullptr || static_cast<uint8>(Screen.Layer) >= static_cast<uint8>(Top->Layer))
		{
			Top = &Screen;
		}
	}

	SetInputMode(Top != nullptr ? Top->InputMode : EAtlasInputMode::Game);
}

void UAtlasUISubsystem::ApplyInputMode(EAtlasInputMode Mode)
{
	APlayerController* PlayerController = GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr;
	if (PlayerController == nullptr)
	{
		return;
	}

	switch (Mode)
	{
	case EAtlasInputMode::Game:
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
		break;

	case EAtlasInputMode::Menu:
		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->SetShowMouseCursor(true);
		break;

	case EAtlasInputMode::GameAndMenu:
		PlayerController->SetInputMode(FInputModeGameAndUI());
		PlayerController->SetShowMouseCursor(true);
		break;

	case EAtlasInputMode::GamepadCursor:
		// Gamepad-driven cursor: UI-only input with the cursor visible; the
		// virtual cursor itself is a Common UI feature configured per-project.
		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->SetShowMouseCursor(true);
		break;
	}

	ATLAS_LOG_UI(Verbose, "Input mode applied: %d", static_cast<int32>(Mode));
}
