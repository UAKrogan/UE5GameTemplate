#include "Widgets/AtlasRootWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Logging/AtlasLogMacros.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UCommonActivatableWidgetStack* UAtlasRootWidget::GetLayerStack(EAtlasUILayer Layer) const
{
	switch (Layer)
	{
	case EAtlasUILayer::Game: return GameLayer;
	case EAtlasUILayer::Menu: return MenuLayer;
	case EAtlasUILayer::Modal: return ModalLayer;
	default:
		ATLAS_ENSURE_MSG(false, "Layer %d is not an activatable stack layer", static_cast<int32>(Layer));
		return nullptr;
	}
}

UOverlay* UAtlasRootWidget::GetLayerOverlay(EAtlasUILayer Layer) const
{
	switch (Layer)
	{
	case EAtlasUILayer::Notification: return NotificationLayer;
	case EAtlasUILayer::Loading: return LoadingLayer;
	default:
		ATLAS_ENSURE_MSG(false, "Layer %d is not an overlay layer", static_cast<int32>(Layer));
		return nullptr;
	}
}

void UAtlasRootWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	const bool bAllLayersBound = GameLayer && MenuLayer && ModalLayer && NotificationLayer && LoadingLayer;
	if (!bAllLayersBound)
	{
		ConstructDefaultLayers();
	}
}

void UAtlasRootWidget::ConstructDefaultLayers()
{
	// Procedural fallback so the UI system works before any Blueprint root
	// widget asset exists. A bound Blueprint hierarchy always wins.
	UOverlay* Root = Cast<UOverlay>(WidgetTree->RootWidget);
	if (Root == nullptr)
	{
		Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
		WidgetTree->RootWidget = Root;
	}

	const auto AddFillSlot = [Root](UWidget* Widget)
	{
		UOverlaySlot* Slot = Root->AddChildToOverlay(Widget);
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Fill);
	};

	// Children added later render on top: order defines the layer Z-order.
	if (GameLayer == nullptr)
	{
		GameLayer = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(
			UCommonActivatableWidgetStack::StaticClass(), TEXT("GameLayer"));
		AddFillSlot(GameLayer);
	}

	if (MenuLayer == nullptr)
	{
		MenuLayer = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(
			UCommonActivatableWidgetStack::StaticClass(), TEXT("MenuLayer"));
		AddFillSlot(MenuLayer);
	}

	if (ModalLayer == nullptr)
	{
		ModalLayer = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(
			UCommonActivatableWidgetStack::StaticClass(), TEXT("ModalLayer"));
		AddFillSlot(ModalLayer);
	}

	if (NotificationLayer == nullptr)
	{
		NotificationLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("NotificationLayer"));
		AddFillSlot(NotificationLayer);
	}

	if (LoadingLayer == nullptr)
	{
		LoadingLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LoadingLayer"));
		AddFillSlot(LoadingLayer);
	}

	ATLAS_LOG_UI(Log, "Root widget layers constructed procedurally");
}
